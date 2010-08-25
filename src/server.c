#include "server.h"
#include "structmember.h"
#include "util.h"

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
    printf("shutdown all.\n");
    loop_done = 0;
}

static inline void 
sigpipe_cb(int signum)
{
}


static inline int
send_writev(write_bucket *data)
{
    size_t w;
    int i = 0;
#ifdef DEBUG
    if(data->binary_protocol){
        //dump binary protocol
    }else{
        printf("writev data=%p iov=%p next=%p\n", data, data->iov, data->next);
        printf("value= %s : %d\n", data->iov[0].iov_base, data->iov[0].iov_len);
        //printf("WRITEV VALUE address=%p 0=%s\n", data->iov[0].iov_base, data->iov[0].iov_base);
        if(data->iov_cnt > 1){
            printf("WRITEV key address=%p key=%s\n", data->iov[1].iov_base, data->iov[1].iov_base);
            printf("WRITEV flag address=%p flags=%s\n", data->iov[2].iov_base, data->iov[2].iov_base);
            if(data->cas){
                printf("WRITEV bytes address=%p bytes=%s\n", data->iov[4].iov_base, data->iov[4].iov_base);
                printf("WRITEV data address=%p data=%s\n", data->iov[5].iov_base, data->iov[5].iov_base);
            }else{
                printf("WRITEV bytes address=%p bytes=%s\n", data->iov[3].iov_base, data->iov[3].iov_base);
                printf("WRITEV data address=%p data=%s\n", data->iov[4].iov_base, data->iov[4].iov_base);
            }
            printf("iov cnt %d \n", data->iov_cnt);
        }
    }
#endif
    w = writev(data->fd, data->iov, data->iov_cnt);
    if(w == -1){
        //error
        if (errno == EAGAIN || errno == EWOULDBLOCK) { 
            /* try again later */
            return 0;
        }else{
            return -1;
        }
    }if(w == 0){
        //is dead
        return -1;
    }else{
        if(data->total > w){
            for(; i < data->iov_cnt;i++){
                if(w > data->iov[i].iov_len){
                    //already write
                    w -= data->iov[i].iov_len;
                    data->iov[i].iov_len = 0;
                }else{
                    data->iov[i].iov_base += w;
                    data->iov[i].iov_len = data->iov[i].iov_len - w;
                    break;
                }
            }
            data->total = data->total -w;
            //resume
            return 0;
        }
    }
    return 1;

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
request_send_data(Client *client, PyObject *env, struct iovec *iov, int iov_cnt, size_t total, bool cas)
{    
    picoev_loop *loop;
    write_bucket *new_bucket, *current;
    ServerObject *server;
    
#ifdef DEBUG
    printf("client fd = %d key_num %d\n", client->fd, client->key_num);
#endif

    server = (ServerObject *)client->server;
    new_bucket = PyMem_Malloc(sizeof(write_bucket));
    memset(new_bucket, 0, sizeof(write_bucket));
    new_bucket->env = env;
    new_bucket->next = NULL;
    new_bucket->fd = client->fd;
    new_bucket->iov = iov;
    new_bucket->iov_cnt = iov_cnt;
    new_bucket->total = total;
    new_bucket->cas = cas;
    new_bucket->binary_protocol = client->binary_protocol;
    loop = server->main_loop;
    bool add =0;
    if(client->data == NULL){
        client->data = new_bucket;
        add = 1;
    }else{
        current = client->data;
        while(1){
            if(current->next){
                current = current->next;
            }else{
                current->next = new_bucket;
                break;
            }
        }
    }
    if(add){
        picoev_add(loop, client->fd, PICOEV_WRITE, TIMEOUT_SECS, write_req_callback, client);
    }
}


/*
static void
timeout_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    Client *client = (Client *)(cb_arg);
    if ((events & PICOEV_TIMEOUT) != 0) {
        //printf("timeout!\n");
        //write_error_response(client, "timeout"); 
        picoev_del(loop, client->fd);
        //send ERROR
        Client_close(client);
    }

}
*/

/*
static void
write_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    Client *client = (Client *)(cb_arg);
    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
        //printf("read timeout! \n");
        write_error_response(client, "timeout"); 
        picoev_del(loop, client->fd);
        Client_close(client);
    
    } else if ((events & PICOEV_WRITE) != 0) {
        switch(client->status){
            case SENDED:
                picoev_set_timeout(loop, client->fd, 1);
                Client_clear(client);                
                break;
            default:
                picoev_del(loop, client->fd);
                Client_close(client);
                break;
        }
    }

}*/


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
    ServerObject *server;
    Client *client;
    
    if ((events & PICOEV_READ) != 0) {
        server = (ServerObject *)cb_arg;

        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd != -1) {
            setup_sock(client_fd);
            remote_addr = inet_ntoa (client_addr.sin_addr);
            remote_port = ntohs(client_addr.sin_port);
            client = (Client *)Client_New((PyObject *)server, client_fd, remote_addr, remote_port);
            if(!client){
                //TODO ERROR
            }
            //client->server = (PyObject *)server;
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
Server_write(ServerObject *self, PyObject *args)
{
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
    write_response((Client *)client, env, response);
    PyDict_SetItemString(env, "done", Py_True);

    Py_RETURN_NONE;

}


static inline int 
inet_listen(ServerObject *server, char *server_name, int server_port)
{
    struct addrinfo hints, *servinfo, *p;
    int flag = 1;
    int rv;
    char strport[7];
    int listen_sock; 
    
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
        if ((listen_sock = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            //perror("server: socket");
            continue;
        }

        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag,
                sizeof(int)) == -1) {
            close(listen_sock);
            PyErr_SetFromErrno(PyExc_IOError);
            return -1;
        }

        if (bind(listen_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_sock);
            PyErr_SetFromErrno(PyExc_IOError);
            return -1;
        }

        break;
    }

    if (p == NULL)  {
        close(listen_sock);
        PyErr_SetString(PyExc_IOError,"server: failed to bind\n");
        return -1;
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    // BACKLOG 1024
    if (listen(listen_sock, BACKLOG) == -1) {
        close(listen_sock);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    server->listen_fd = listen_sock;
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
unix_listen(ServerObject *server, char *sock_name)
{
    int flag = 1;
    struct sockaddr_un saddr;
    int listen_sock;
    mode_t old_umask;

#ifdef DEBUG
    printf("unix domain socket %s\n", sock_name);
#endif
    memset(&saddr, 0, sizeof(saddr));
    check_unix_sockpath(sock_name);

    if ((listen_sock = socket(AF_UNIX, SOCK_STREAM,0)) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag,
            sizeof(int)) == -1) {
        close(listen_sock);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }

    saddr.sun_family = PF_UNIX;
    strcpy(saddr.sun_path, sock_name);
    
    old_umask = umask(0);

    if (bind(listen_sock, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        close(listen_sock);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    umask(old_umask);

    // BACKLOG 1024
    if (listen(listen_sock, BACKLOG) == -1) {
        close(listen_sock);
        PyErr_SetFromErrno(PyExc_IOError);
        return -1;
    }
    server->listen_fd = listen_sock;
    server->unix_sock_name = sock_name;
    return 1;
}


static inline PyObject * 
Server_listen(ServerObject *self, PyObject *args)
{
    char *server_name;
    int server_port;
    PyObject *o;
    int ret;

    if (!PyArg_ParseTuple(args, "O:listen", &o))
        return NULL;

    if(self->listen_fd > 0){
        PyErr_SetString(PyExc_Exception, "already set listen socket");
        return NULL;
    }
    
    if(PyTuple_Check(o)){
        //inet 
        if(!PyArg_ParseTuple(o, "si:listen", &server_name, &server_port))
            return NULL;
        ret = inet_listen(self, server_name, server_port);
    }else if(PyString_Check(o)){
        // unix domain 
        ret = unix_listen(self, PyString_AS_STRING(o));
    }else{
        PyErr_SetString(PyExc_TypeError, "args tuple or string(path)");
        return NULL;
    }
    if(ret < 0){
        //error 
        self->listen_fd = -1;
        return NULL;
    }

    Py_RETURN_NONE;
}


static inline PyObject *
Server_run(ServerObject *self){

    picoev_loop* main_loop; //main loop

    picoev_init(MAX_FDS);
    /* create loop */
    main_loop = picoev_create_loop(60);
    loop_done = 1;
    setsig(SIGPIPE, sigpipe_cb);
    setsig(SIGINT, sigint_cb);
    
    self->main_loop = main_loop;
    
    setup_listen_sock(self->listen_fd);

    /* add listen socket */
    picoev_add(main_loop, self->listen_fd, PICOEV_READ, 0, accept_callback, self);
    /* loop */
    while (loop_done) {
        picoev_loop_once(main_loop, 10);
    }
    
    picoev_destroy_loop(main_loop);
    picoev_deinit();
    
    Py_RETURN_NONE;
}

static PyMemberDef Server_members[] = {
    {NULL} 
};



static PyMethodDef Server_method[] = {
    { "listen",      (PyCFunction)Server_listen, METH_VARARGS, 0 },
    { "run",      (PyCFunction)Server_run, METH_VARARGS, 0 },
    { "write",      (PyCFunction)Server_write, METH_VARARGS, 0 },
    //{ "MemClient",      (PyCFunction)Server_memclient, METH_VARARGS, 0 },
    { NULL, NULL}
};

PyTypeObject ServerType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "mgate.Server",             /*tp_name*/
    sizeof(ServerObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Sever",                   /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Server_method,             /* tp_methods */
    Server_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,     /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                         /* tp_new */
};

static PyMethodDef MGateMethods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initmgate(void)
{
    PyObject *m;
    
    if(PyType_Ready(&ServerType) < 0)
        return;
    if(PyType_Ready(&ClientType) < 0)
        return;
    
    m = Py_InitModule3("mgate", MGateMethods, "");
    if(m == NULL)
        return;

    Py_INCREF(&ServerType);
    PyModule_AddObject(m, "Server", (PyObject *)&ServerType);

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
