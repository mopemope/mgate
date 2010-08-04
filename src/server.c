#include "server.h"
#include "structmember.h"


#define MAX_FDS 1024 * 4
#define TIMEOUT_SECS 1

#define BACKLOG 1024 * 4
#define MAX_BUFSIZE 8192

char *stored_response[] = {
    STORED,
    NOT_STORED,
    EXISTS,
    NOT_FOUND
};

static void
read_callback(picoev_loop* loop, int fd, int events, void* cb_arg);

//static void
//write_callback(picoev_loop* loop, int fd, int events, void* cb_arg);

int loop_done = 0;

static 
void setup_sock(int fd)
{
    int on = 1, r;
    r = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    assert(r == 0);
    r = fcntl(fd, F_SETFL, O_NONBLOCK);
    assert(r == 0);
}

static int
setsig(int sig, void* handler)
{
    struct sigaction context, ocontext;
    context.sa_handler = handler;
    sigemptyset(&context.sa_mask);
    context.sa_flags = 0;
    return sigaction(sig, &context, &ocontext);
}

static void 
sigint_cb(int signum)
{
    printf("Bye.\n");
    loop_done = 0;
}

static void 
sigpipe_cb(int signum)
{
}


static int
send_writev(write_data *data)
{
    size_t w;
    int i = 0;
#ifdef DEBUG
    printf("writev data=%p iov=%p netx=%p\n", data, data->iov, data->next);
    printf("WRITEV VALUE address=%p key=%s\n", data->iov[0].iov_base, data->iov[0].iov_base);
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
        return 1;
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
            //return send_writev(fd, iov, iov_cnt, total - w); 
        }
    }
    return 1;

}

static void
clear_write_data(write_data *data)
{
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


static void
write_req_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    int ret;
    Client *client = (Client *)(cb_arg);
    write_data *data = client->data;

#ifdef DEBUG
    printf("write callback %d\n", fd);
#endif
    
    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
#ifdef DEBUG
        printf("write callback timeout %d\n", fd);
#endif
        write_error_response(client, "timeout"); 
        picoev_del(loop, fd);
        clear_write_data(data);
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
                clear_write_data(data);
                Client_close(client);
                
                break;
            default:
                //ok
                if(data->next){
                    write_data *next = data->next;
                    data->next = NULL;
                    clear_write_data(data);
                    client->data = next;
                }else{
                    client->data = NULL;
                    clear_write_data(data);
                }
                if(client->data == NULL){
#ifdef DEBUG
#endif
                    //send END
                    Client_clear(client);
                    picoev_del(loop, fd);
                    picoev_add(loop, fd, PICOEV_READ, TIMEOUT_SECS, read_callback, client);
                }

        };

    }

}


void
request_send_data(Client *client, PyObject *env, struct iovec *iov, int iov_cnt, size_t total, bool cas)
{    
    picoev_loop *loop;
    write_data *new_data;
    ServerObject *server;
    
#ifdef DEBUG
    //printf("key_num %d\n", client->key_num);
    //printf("client fd = %d key_num %d\n", fd, client->key_num);
#endif
    server = (ServerObject *)client->server;
    new_data = PyMem_Malloc(sizeof(write_data));
    memset(new_data, 0, sizeof(write_data));
    new_data->env = env;
    new_data->next = NULL;
    new_data->fd = client->fd;
    new_data->iov = iov;
    new_data->iov_cnt = iov_cnt;
    new_data->total = total;
    new_data->cas = cas;
    loop = server->main_loop;
    bool add =0;
    if(client->data == NULL){
        client->data = new_data;
        add = 1;
    }else{
        write_data *current;
        current = client->data;
        while(1){
            if(current->next){
                current = current->next;
            }else{
                current->next = new_data;
                break;
            }
        }
    }
    if(add){
        picoev_add(loop, client->fd, PICOEV_WRITE, TIMEOUT_SECS, write_req_callback, client);
    }
}



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


static void
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
        picoev_set_timeout(loop, client->fd, 1);
        char buf[MAX_BUFSIZE];
        ssize_t r;
        r = read(fd, buf, sizeof(buf));
        switch (r) {
            /*
            case 0: 
#ifdef DEBUG
                printf("read callback close %d\n", fd);
#endif
                picoev_del(loop, client->fd);
                Client_close(client);

                break;
            */
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
                }
                break;
            default: 
                Client_exec_parse(client, buf, r);
                break;
        }

    }
}


static void
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

static PyObject * 
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

static PyObject * 
Server_listen(ServerObject *self, PyObject *args)
{
    char *server_name;
    int server_port;
    struct addrinfo hints, *servinfo, *p;
    int flag = 1;
    int rv;
    char strport[7];
    int listen_fd;
    
    if(!PyArg_ParseTuple(args, "si", &server_name, &server_port)){
        //TODO
        return NULL;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol= IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE; // use my IP
    
    snprintf(strport, sizeof (strport), "%d", server_port);
    
    if ((rv = getaddrinfo(server_name, strport, &hints, &servinfo)) == -1) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((listen_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    // BACKLOG 1024
    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    setup_sock(listen_fd);
    //printf("listen on %s:%i\n", server_name, server_port);
    self->listen_fd = listen_fd;
    Py_RETURN_NONE;
}


static PyObject *
Server_run(ServerObject *self){

    picoev_loop* main_loop; //main loop

    picoev_init(MAX_FDS);
    /* create loop */
    main_loop = picoev_create_loop(60);
    loop_done = 1;
    setsig(SIGPIPE, sigpipe_cb);
    setsig(SIGINT, sigint_cb);
    
    self->main_loop = main_loop;

    /* add listen socket */
    picoev_add(main_loop, self->listen_fd, PICOEV_READ, 0, accept_callback, self);
    /* loop */
    while (loop_done) {
        if(picoev_loop_once(main_loop, 10) < 0){
            //loop error
            loop_done = 0;
        }
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
    PyVarObject_HEAD_INIT(NULL, 0)
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
