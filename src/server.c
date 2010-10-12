#include "server.h"
#include "structmember.h"
#include "util.h"
#include "parser/common.h"
#include "response/response.h"

#define MAX_FDS 1024 * 8
#define TIMEOUT_SECS 1

#define BACKLOG 1024 * 8
#define MAX_BUFSIZE 8192

char *stored_response[] = {
    STORED,
    NOT_STORED,
    EXISTS,
    NOT_FOUND
};

static char *server_name = "127.0.0.1";
static short server_port = 8000;
static int listen_sock;  // listen socket

picoev_loop* main_loop; //main loop

PyObject *mgate_app = NULL; //mgte app
static PyObject *watchdog = NULL; //watchdog
static char *log_path = NULL; //access log path
static int log_fd = -1; //access log
static char *error_log_path = NULL; //error log path
static int err_log_fd = -1; //error log

int max_content_length = 1024 * 1024 * 16; //max_content_length

static char *unix_sock_name = NULL;

static void
read_callback(picoev_loop* loop, int fd, int events, void* cb_arg);

int loop_done = 0;

static inline int
setsig(int sig, void* handler)
{
    struct sigaction context, ocontext;
    context.sa_handler = handler;
    sigemptyset(&context.sa_mask);
    context.sa_flags = 0;
    return sigaction(sig, &context, &ocontext);
}

static inline void 
sigint_cb(int signum)
{
    //printf("shutdown all.\n");
    loop_done = 0;
}

static inline void 
sighup_cb(int signum)
{

}

static inline void 
sigpipe_cb(int signum)
{

}


static inline void
clear_write_bucket(write_bucket *data)
{
    if(data->binary_protocol){
    
    }else{
        if(data && data->iov){

            if(data->iov_cnt > 6){
                PyMem_Free(data->iov[2].iov_base);
                PyMem_Free(data->iov[3].iov_base);
            }
            
            if(data->cas){
                PyMem_Free(data->iov[4].iov_base);
            }
            
            Py_XDECREF(data->env);
            PyMem_Free(data->iov);
            data->iov = NULL;
        }
        data->next = NULL;
        if(data){
            PyMem_Free(data);
            data = NULL;
        }
    }


}


static inline void
write_req_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    int ret;
    Client *client = (Client *)(cb_arg);
    write_bucket *data = client->data;

#ifdef DEBUG
    printf("write callback fd:%d\n", fd);
#endif
    
    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
#ifdef DEBUG
        printf("write callback timeout %d\n", fd);
#endif
        write_error_response(client, "timeout"); 
        picoev_del(loop, fd);
        clear_write_bucket(data);
        Client_close(client);
    
    } else if ((events & PICOEV_WRITE) != 0) {
        picoev_set_timeout(loop, fd, 1);
        ret = send_writev(data);
        switch(ret){
            case 0:
                // try again
                break;
            case -1:
                //send fatal error
                picoev_del(loop, fd);
                clear_write_bucket(data);
                Client_close(client);
                
                break;
            default:
                //ok
                if(data->next){
                    //check next data
                    write_bucket *next = data->next;
                    data->next = NULL;
                    clear_write_bucket(data);
                    //switch new data
                    client->data = next;
                }else{
                    client->data = NULL;
                    clear_write_bucket(data);
                }
                if(client->data == NULL){
                    //send END
                    //all done
                    Client_clear(client);
                    picoev_del(loop, fd);
                    picoev_add(loop, fd, PICOEV_READ, TIMEOUT_SECS, read_callback, client);
                }

        };

    }

}

inline void
send_bucket(Client *client, write_bucket *bucket)
{
    uint8_t add;
    write_bucket *current;

    if(client->data == NULL){
        client->data = bucket;
        add = 1;
    }else{
        current = client->data;
        while(1){
            if(current->next){
                current = current->next;
            }else{
                current->next = bucket;
                break;
            }
        }
    }

    if(add){
        picoev_add(main_loop, client->fd, PICOEV_WRITE, TIMEOUT_SECS, write_req_callback, client);
    }
}



static inline void
read_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    Client *client = (Client *)(cb_arg);

    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
#ifdef DEBUG
        printf("read callback timeout %d\n", fd);
#endif
        write_error_response(client, "timeout"); 
        picoev_del(loop, client->fd);
        Client_close(client);
    
    } else if ((events & PICOEV_READ) != 0) {

        /* update timeout, and read */
        picoev_set_timeout(loop, client->fd, TIMEOUT_SECS);
        char buf[MAX_BUFSIZE];
        ssize_t r;
        int parse_result;
        r = read(fd, buf, sizeof(buf));
        switch (r) {
            case 0: 
#ifdef DEBUG
                printf("read callback close %d\n", fd);
#endif
                picoev_del(loop, client->fd);
                Client_close(client);

                return;
            case -1: /* error */
                if (errno == EAGAIN || errno == EWOULDBLOCK) { /* try again later */
#ifdef DEBUG
                    printf("read callback EAGAIN %d\n", fd);
#endif
	                break;
                } else { /* fatal error */
#ifdef DEBUG
                    printf("read callback fatal error %d\n", fd);
#endif
                    write_error_response(client, "i/o error"); 
                    picoev_del(loop, client->fd);
                    Client_close(client);
                    return;
                }
                break;
            default: 
                parse_result = Client_exec_parse(client, buf, r);
                // TODO check value
                break;
        }

    }
}


static inline void
accept_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    char *remote_addr;
    int remote_port;
    Client *client;
    
    if ((events & PICOEV_READ) != 0) {

        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd != -1) {
            setup_sock(client_fd);
            remote_addr = inet_ntoa (client_addr.sin_addr);
            remote_port = ntohs(client_addr.sin_port);
            client = (Client *)Client_New(client_fd, remote_addr, remote_port);
            if(!client){
                //TODO Error
                return ;
            }
            picoev_add(loop, client_fd, PICOEV_READ, TIMEOUT_SECS, read_callback, client);
        }else{
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                PyErr_SetFromErrno(PyExc_IOError);
                // die
                loop_done = 0;
            }
        }
    }
}

/* access proxyobject */
typedef struct {
	PyObject_HEAD
	PyObject *dict;
} proxyobject;

/*
static PyObject *
Server_memclient(ServerObject *self, PyObject *args)
{
    PyObject *list;

    if(!PyArg_ParseTuple(args, "O:memclient", &list)){
        return NULL;
    }
    PyObject *memclient = MemClient_New(self, list);
    return memclient;

}*/

static inline PyObject * 
mgate_write(PyObject *self, PyObject *args)
{
    int ret;
    PyObject *client;
    PyObject *pe;

    PyObject *response;
    PyObject *env;
    //PyObject *flag;

    if(!PyArg_ParseTuple(args, "OO:write", &pe, &response)){
        return NULL;
    }

    //TODO type check
    
    env = ((proxyobject *)pe)->dict;
    if(PyDict_GetItemString(env, "done")){
        
        PyErr_SetString(PyExc_IOError,"This data is already sent");
        return NULL;
    }
    client = PyDict_GetItemString(env, "_client");
    
    ret = write_response((Client *)client, env, response);
    
    if(ret < 0){
        //already set PyErr
        return NULL;
    }

    //mark sended
    PyDict_SetItemString(env, "done", Py_True);

    Py_RETURN_NONE;

}


static inline int 
inet_listen(char *server_name, int server_port)
{
    struct addrinfo hints, *servinfo, *p;
    int flag = 1;
    int rv;
    char strport[7];
    int listen_fd; 
    
    memset(&hints, 0, sizeof hints);
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    
    snprintf(strport, sizeof (strport), "%d", server_port);
    
    if ((rv = getaddrinfo(server_name, strport, &hints, &servinfo)) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((listen_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            //perror("server: socket");
            continue;
        }

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag,
                sizeof(int)) == -1) {
            close(listen_fd);
            PyErr_SetFromErrno(PyExc_IOError);
            return -1;
        }

        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_fd);
            PyErr_SetFromErrno(PyExc_IOError);
            return -1;
        }

        break;
    }

    if (p == NULL)  {
        close(listen_fd);
        PyErr_SetString(PyExc_IOError,"server: failed to bind\n");
        return -1;
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    // BACKLOG 1024
    if (listen(listen_fd, BACKLOG) == -1) {
        close(listen_fd);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    listen_sock = listen_fd;
    return 1;
}

static inline int
check_unix_sockpath(char *sock_name)
{
    if(!access(sock_name, F_OK)){
        if(unlink(sock_name) < 0){
            PyErr_SetFromErrno(PyExc_IOError);
            return -1;
        }
    }
    return 1;
}

static inline int
unix_listen(char *sock_name)
{
    int flag = 1;
    struct sockaddr_un saddr;
    int listen_fd;
    mode_t old_umask;

#ifdef DEBUG
    printf("unix domain socket %s\n", sock_name);
#endif
    memset(&saddr, 0, sizeof(saddr));
    check_unix_sockpath(sock_name);

    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM,0)) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag,
            sizeof(int)) == -1) {
        close(listen_fd);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    saddr.sun_family = PF_UNIX;
    strcpy(saddr.sun_path, sock_name);
    
    old_umask = umask(0);

    if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        close(listen_fd);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    umask(old_umask);

    // BACKLOG 1024
    if (listen(listen_fd, BACKLOG) == -1) {
        close(listen_fd);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    listen_sock = listen_fd;
    unix_sock_name = sock_name;
    return 1;
}


static inline PyObject * 
mgate_listen(PyObject *self, PyObject *args)
{
    PyObject *o;
    int ret;

    if (!PyArg_ParseTuple(args, "O:listen", &o))
        return NULL;

    if(listen_sock > 0){
        PyErr_SetString(PyExc_Exception, "already set listen socket");
        return NULL;
    }
    
    if(PyTuple_Check(o)){
        //inet 
        if(!PyArg_ParseTuple(o, "si:listen", &server_name, &server_port)){
            return NULL;
        }

        ret = inet_listen(server_name, server_port);
    }else if(PyString_Check(o)){
        // unix domain 
        ret = unix_listen(PyString_AS_STRING(o));
    }else{
        PyErr_SetString(PyExc_TypeError, "args tuple or string(path)");
        return NULL;
    }
    if(ret < 0){
        //error 
        listen_sock = -1;
        return NULL;
    }

    Py_RETURN_NONE;
}


static inline PyObject *
mgate_run(PyObject *self, PyObject *args)
{

    int i = 0;
    PyObject *watchdog_result;

    if (!PyArg_ParseTuple(args, "O:run", &mgate_app))
        return NULL; 
    
    if(listen_sock <= 0){
        PyErr_Format(PyExc_TypeError, "not found listen socket");
        return NULL;
        
    }
    
    if(!PyCallable_Check(mgate_app)){
        PyErr_SetString(PyExc_TypeError, "must be callable");
        mgate_app = NULL;
        return NULL;
    }

    Py_INCREF(mgate_app);
    

    picoev_init(MAX_FDS);
    /* create loop */
    main_loop = picoev_create_loop(60);
    loop_done = 1;
    setsig(SIGPIPE, sigpipe_cb);
    setsig(SIGINT, sigint_cb);
    setsig(SIGHUP, sighup_cb);
    
    setup_listen_sock(listen_sock);
    setup_env_key();

    /* add listen socket */
    picoev_add(main_loop, listen_sock, PICOEV_READ, 0, accept_callback, NULL);
    /* loop */
    while (loop_done) {
        picoev_loop_once(main_loop, 10);
        i++;
        // watchdog slow.... skip check
        
        //if(watchdog && i > 1){
        if(watchdog){
            watchdog_result = PyObject_CallFunction(watchdog, NULL);
            if(PyErr_Occurred()){
                PyErr_Print();
                PyErr_Clear();
            }
            Py_XDECREF(watchdog_result);
            i = 0;
        //}else if(tempfile_fd){
        //    fast_notify();
        }
    }

    Py_DECREF(mgate_app);
    Py_XDECREF(watchdog);
    
    picoev_destroy_loop(main_loop);
    picoev_deinit();
    
    //clean
    //clear_start_response();
    clear_env_key();

    //Py_DECREF(hub_switch_value);
    //Py_DECREF(client_key);
    //Py_DECREF(wsgi_input_key);
    //Py_DECREF(empty_string);

    if(unix_sock_name){
        unlink(unix_sock_name);
    }
    Py_RETURN_NONE;
}


static PyMethodDef MGateMethods[] = {
    { "listen",      (PyCFunction)mgate_listen, METH_VARARGS, 0 },
    { "run",      (PyCFunction)mgate_run, METH_VARARGS, 0 },
    { "write",      (PyCFunction)mgate_write, METH_VARARGS, 0 },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initmgate(void)
{
    PyObject *m;
    
    
    if(PyType_Ready(&ClientType) < 0)
        return;
    
    m = Py_InitModule3("mgate", MGateMethods, "");
    if(m == NULL)
        return;

	if (PyModule_AddStringConstant(m, "STORED",
				       stored_response[0]) == -1)
        return;
	if (PyModule_AddStringConstant(m, "NOT_STORED",
				       stored_response[1]) == -1)
        return;
	if (PyModule_AddStringConstant(m, "EXISTS",
				       stored_response[2]) == -1)
        return;
	if (PyModule_AddStringConstant(m, "NOT_FOUND",
				       stored_response[3]) == -1)
        return;
	if (PyModule_AddStringConstant(m, "DELETED",
				       DELETED) == -1)
        return;

}
