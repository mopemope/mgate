#include "server.h"
#include "parser/text_parser.h"
#include "parser/binary_parser.h"
#include "response.h"

#define BUFSIZE 8192

int 
write_retrieval(Client *client, PyObject *env, PyObject *response, unsigned short flags, uint64_t cas_unique);

void 
write_error_response(Client *client, char *msg)
{
    text_error_response(client, msg);
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

static void
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
    server = (ServerObject *)pyclient->server;
}

static void
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

int 
write_numeric(Client *client, PyObject *env, PyObject *response)
{
    PyObject *str_response = NULL;
    char *data;
    Py_ssize_t data_len;
    int ret;

    if(PyInt_Check(response)){
        str_response = PyObject_Str(response);
    //}else if(PyString_Check(response)){
    //    str_response = response;
    }else if(PyBool_Check(response)){
        //BOOL
        if(!PyObject_IsTrue(response)){
            str_response = PyString_FromString(NOT_FOUND); 
        }else{
            //???
            goto error;
        }
    }else{
        goto error;
    }

    if(PyString_AsStringAndSize(str_response, &data, &data_len)){
        //TODO raise Error
        goto error;
    }
    ret = text_numeric_response(client, env, data, data_len);
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return ret;
    
error:
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return 0;
}

int 
write_delete(Client *client, PyObject *env, PyObject *response)
{
    PyObject *str_response = NULL;
    char *data;
    Py_ssize_t data_len;
    int ret;
    
    if(PyBool_Check(response)){
        //BOOL
        if(PyObject_IsTrue(response)){
            str_response = PyString_FromString(DELETED);
        }else{
            str_response = PyString_FromString(NOT_FOUND); 
        }
    }else if(PyString_Check(response)){
        str_response = response;
        Py_INCREF(str_response);
    }else{
        //TODO error

        goto error;
    }

    
    if(PyString_AsStringAndSize(str_response, &data, &data_len)){
        //TODO raise Error
        goto error;
    }
    ret = text_simple_response(client, env, data, data_len);
    if(ret < 0){
        //write_error
        //raise Error
        goto error;
    }
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return ret;
error:
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return 0;

}

int 
write_storage(Client *client, PyObject *env, PyObject *response)
{
    PyObject *str_response = NULL;
    char *data;
    Py_ssize_t data_len;
    int ret;

#ifdef DEBUG
    printf("call write_storage");
#endif

    if(PyBool_Check(response)){
        //BOOL
        if(PyObject_IsTrue(response)){
            str_response = PyString_FromString(STORED);
        }else{
            str_response = PyString_FromString(NOT_STORED); 
        }
    }else if(PyString_Check(response)){
        str_response = response;
        //Py_INCREF(str_response);
    }else{
        //TODO error

        goto error;
    }

    
    if(PyString_AsStringAndSize(str_response, &data, &data_len)){
        //TODO raise Error
        goto error;
    }
    ret = text_simple_response(client, env, data, data_len);
    if(ret < 0){
        //write_error
        //raise Error
        goto error;
    }
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return ret;
error:
    Py_XDECREF(str_response);
    //Py_DECREF(response);
    return 0;

}

int 
write_retrieval_flags(Client *client, PyObject *env, PyObject *response)
{
    int ret;
    int fd;
    
    PyObject *pcmd = PyDict_GetItemString(env, "cmd");
    memtext_command cmd = (memtext_command)PyInt_AsLong(pcmd);
    fd = client->fd;

    if(PyTuple_Check(response)){
        if(cmd == MEMTEXT_CMD_GET){
            PyObject *data = PyTuple_GetItem(response, 0);
            PyObject *flags = PyTuple_GetItem(response, 1);
            if(PyErr_Occurred()){
                Py_XDECREF(response);
                return 0;
            }
            int c_flags = PyInt_AsLong(flags);
#ifdef DEBUG
            printf("flasg %d \n", c_flags);
#endif
            if(PyErr_Occurred()){
                Py_XDECREF(response);
                return 0;
            }

            Py_INCREF(data);
            ret = write_retrieval(client, env, data, c_flags, 0);
            //Py_XDECREF(flags);
            //Py_XDECREF(response);
        }else{
            PyObject *data = PyTuple_GetItem(response, 0);
            PyObject *flags = PyTuple_GetItem(response, 1);
            PyObject *cas = PyTuple_GetItem(response, 2);
            if(PyErr_Occurred()){
                Py_XDECREF(response);
                return 0;
            }
            int c_flags = PyInt_AsLong(flags);
            uint64_t c_cas = PyInt_AsLong(cas);
            if(PyErr_Occurred()){
                Py_XDECREF(response);
                return 0;
            }
            //Py_XDECREF(response);
            Py_INCREF(data);
            ret = write_retrieval(client, env, data, c_flags, c_cas);
            //Py_XDECREF(flags);
            //Py_XDECREF(response);
        
        }

    }else{
        ret = write_retrieval(client, env, response, 0, 0);
    }
    return ret;
}

int 
write_retrieval(Client *client, PyObject *env, PyObject *response, unsigned short flags, uint64_t cas_unique)
{
#ifdef DEBUG
    printf("call write_retrieval \n");
#endif
    PyObject *keyobj;
    char *key, *data;
    Py_ssize_t key_len, data_len;
    int ret;
    int fd;
    
    fd = client->fd;

    if(!PyString_Check(response)){
        //TODO raise Error
        goto error;
    }

    //get key data
    keyobj = PyDict_GetItemString(env, "key");
    if(!keyobj){
        goto error;
    }


    if(PyString_AsStringAndSize(keyobj, &key, &key_len)){
        //TODO raise Error
        goto error;
    }
    
    //get response data
    if(PyString_AsStringAndSize(response, &data, &data_len)){
        //TODO raise Error
        goto error;
    }
    
#ifdef DEBUG
    printf("request_retrieval \n");
#endif

    ret = text_get_response(client, env, key, key_len, data, data_len, flags, cas_unique);
    if(ret < 0){
        //write_error
        //raise Error
        goto error;
    }
    Py_DECREF(response);
    return ret;
error:
    Py_DECREF(response);
    return 0;
}

int 
write_response(Client *self, PyObject *env, PyObject *response)
{
    int ret = -1;
     
    PyObject *pcmd = PyDict_GetItemString(env, "cmd");

    memtext_command cmd = (memtext_command)PyInt_AsLong(pcmd);

    switch(cmd){
        /*retrieval*/    
        case MEMTEXT_CMD_GET:
            ret = write_retrieval_flags(self, env, response);
            break;
        case MEMTEXT_CMD_GETS:
            ret = write_retrieval_flags(self, env, response);
            break;
        /* storage */
        case MEMTEXT_CMD_SET:
        case MEMTEXT_CMD_ADD:
        case MEMTEXT_CMD_REPLACE:
        case MEMTEXT_CMD_APPEND:
        case MEMTEXT_CMD_PREPEND:
            ret = write_storage(self, env, response);
            break;
        /* cas */
        case MEMTEXT_CMD_CAS:
            ret = write_storage(self, env, response);
            break;
        /* delete */
        case MEMTEXT_CMD_DELETE:
            ret = write_delete(self, env, response);
            break;
        /* numeric */
        case MEMTEXT_CMD_INCR:
        case MEMTEXT_CMD_DECR:
            ret = write_numeric(self, env, response);
            break;
        /* other */
        case MEMTEXT_CMD_VERSION:
            break;
        default:
            break;
    }
    return ret;

}

void
Client_exec_parse(Client *self, char *buf, size_t read_length)
{
    client_t *client;
    ServerObject *server;
    client = self->client;
    server = self->server;

    buf_write(client, buf, read_length);
    
    if(!client->parser){
        //
        if(buf[0] == 0x80){
            init_binary_parser(self);
            self->binary_protocol = 1;
        }else{
            init_text_parser(self);
        }
    }

    if(self->binary_protocol){
        execute_binray_parse(self, client->input_buf, client->input_len, &(client->input_pos));
    }else{
        execute_text_parse(self, client->input_buf, client->input_len, &(client->input_pos));
    }
}

void 
Client_clear(Client *self)
{
#ifdef DEBUG
    printf("clear fd = %d\n", self->fd);
#endif
    ServerObject *server = (ServerObject *)self->server;
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


void
Client_close(Client *self)
{
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


PyObject *  
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
    self->binary_protocol = ((ServerObject *)server)->binary_protocol;
    client_t_new(self, fd, remote_addr, remote_port);
    return (PyObject *)self;
}

static void
Client_dealloc(Client* self)
{
    self->server = NULL;
    self->client = NULL;
    self->key_num = 0;
    self->data = NULL;
    PyObject_DEL(self);
}



PyTypeObject ClientType = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
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
