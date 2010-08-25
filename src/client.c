#include "server.h"
#include "parser/text_parser.h"
#include "parser/binary_parser.h"
#include "response/text_response.h"
#include "response/binary_response.h"

#define BUFSIZE 8192


inline void 
write_error_response(Client *client, char *msg)
{
    if(client->binary_protocol){
        binary_error_response(client, msg);
    }else{
        text_error_response(client, msg);
    }
}


static inline int
buf_write(client_t *client, const char *c, size_t  l) {
    size_t newl;
    char *newbuf;
    newl = client->input_len + l;
    if (newl >= client->input_buf_size) {
        client->input_buf_size *= 2;
        if (client->input_buf_size <= newl) {
            //assert(newl + 1 < INT_MAX);
            client->input_buf_size = (int)(newl + 1);
        }
        newbuf = (char*)realloc(client->input_buf, client->input_buf_size);
        if (!newbuf) {
            printf("out of memory\n");
            PyErr_SetString(PyExc_MemoryError,"out of memory");
            free(client->input_buf);
            client->input_buf = 0;
            client->input_buf_size = client->input_len = 0;
            return -1;
        }
        client->input_buf = newbuf;
    }

    memcpy(client->input_buf + client->input_len, c , l);

    client->input_len += (int)l;
    return (int)l;
}

static inline void
client_t_new(Client *pyclient, int fd, char *remote_addr, int remote_port)
{
    client_t *client;
    ServerObject *server;
    client = PyMem_Malloc(sizeof(client_t)); 
    memset(client, 0, sizeof(client_t));
    client->fd = fd;
    client->input_buf = malloc(sizeof(char) * BUFSIZE);
    client->input_buf_size = sizeof(char) * BUFSIZE;
    client->remote_addr = remote_addr;
    client->remote_port = remote_port;
    pyclient->client = client;
}

static inline void
free_client_field(client_t *client)
{

    if(client->parser != NULL){
        PyMem_Free(client->parser);
        client->parser = NULL;
    }

    if(client->callback != NULL){
        PyMem_Free(client->callback);
        client->callback = NULL;
    }

    if(client->input_buf != NULL){
        free(client->input_buf);
        client->input_buf = NULL;
    }
    
}


inline int 
write_response(Client *self, PyObject *env, PyObject *response)
{
    
    if(!self->tcp_cork){
        //cork
        enable_cork(self->fd);
        self->tcp_cork = 1;
    }
    if(self->binary_protocol){
        return write_binary_response(self, env, response);
    }else{
        return write_text_response(self, env, response);
    }

}


inline int 
Client_exec_parse(Client *self, char *buf, size_t read_length)
{
    client_t *client;
    ServerObject *server;
    client = self->client;
    server = (ServerObject *)self->server;

    buf_write(client, buf, read_length);
    
    if(!client->parser){
        //
        if(buf[0] == 0x80){
            init_binary_parser(self);
            self->binary_protocol = 1;
#ifdef DEBUG
            printf("use binary protocol \n");
#endif
        }else{
            init_text_parser(self);
            self->binary_protocol = 0;
#ifdef DEBUG
            printf("use text protocol");
#endif
        }
    }

    if(self->binary_protocol){
        return execute_binray_parse(self, client->input_buf, client->input_len, &(client->input_pos));
    }else{
        return execute_text_parse(self, client->input_buf, client->input_len, &(client->input_pos));
    }
}

inline void 
Client_clear(Client *self)
{
#ifdef DEBUG
    printf("clear fd = %d\n", self->fd);
#endif
    client_t *client;
    client = self->client;
    free_client_field(client);
    client->input_buf = malloc(sizeof(char) * BUFSIZE);
    client->input_buf_size = sizeof(char) * BUFSIZE;
    client->input_pos = 0;
    client->input_len = 0;
    self->key_num = 0;
    self->data = NULL;
    //PyDict_Clear(self->env);
    self->binary_protocol = 0;
}


inline void
Client_close(Client *self)
{
    disable_cork(self->fd);
    client_t *client = self->client;
    free_client_field(client);
    PyMem_Free(client);
    close(self->fd);
    //PyDict_Clear(self->env);
    //Py_XDECREF(self->env);
    Py_DECREF(self);
#ifdef DEBUG
    printf("close fd = %d\n", self->fd);
#endif

}


inline PyObject *  
Client_New(PyObject *server, int fd, char *remote_addr, int remote_port)
{
    Client *self;

    self = PyObject_NEW(Client, &ClientType);
    if(self == NULL){
        return NULL;
    }
    self->fd = fd;
    self->key_num = 0;
    self->data = NULL;
    self->server = server;
    client_t_new(self, fd, remote_addr, remote_port);
    return (PyObject *)self;
}

static inline void
Client_dealloc(Client* self)
{
    self->server = NULL;
    self->client = NULL;
    self->key_num = 0;
    self->data = NULL;
    PyObject_DEL(self);
}



PyTypeObject ClientType = {
	PyObject_HEAD_INIT(&PyType_Type)
    0,
    "mgate.client",             /*tp_name*/
    sizeof(Client), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Client_dealloc, /*tp_dealloc*/
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
    0, /*ResponseObject_str*/                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "memcached request client ",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                      /* tp_init */
    0,                         /* tp_alloc */
    0,                           /* tp_new */
};

