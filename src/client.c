#include "server.h"
#include "parser.h"
#include "b_parser.h"

#define BUFSIZE 4096


static int
request_retrieval(Client *client, PyObject *env, char *key, size_t key_len, char *data, size_t data_len, unsigned short flags, uint64_t cas_unique);

int 
write_retrieval(Client *client, PyObject *env, PyObject *response, unsigned short flags, uint64_t cas_unique);

static void
send_server_error(Client *client, char *error)
{
    struct iovec iov[3];

    iov[0].iov_base = "SERVER_ERROR ";
    iov[0].iov_len = 13;

    iov[1].iov_base = error;
    iov[1].iov_len = strlen(error);

    iov[2].iov_base = "\r\n";
    iov[2].iov_len = 2;
    
    if(writev(client->fd, iov, 3) < 0){
        //error
        //printf("send error!\n");
        return;
    }
    //printf("error %s \n" , error);
    
}

void 
write_error_response(Client *client, char *msg)
{
    send_server_error(client, msg);
}

static int 
request_simple(Client *client, PyObject *env, char *data, size_t data_len)
{

    size_t total = 0;
    struct iovec *iov;
    iov = (struct iovec *)PyMem_Malloc(sizeof(struct iovec) * 1);
 
    iov[0].iov_base = data;
    iov[0].iov_len = data_len;
    total += iov[0].iov_len;

    request_send_data(client, env, iov, 1, total, false);

    return 1;
}

static int 
request_numeric(Client *client, PyObject *env, char *data, size_t data_len)
{
    size_t total = 0;
    struct iovec *iov;
    iov = (struct iovec *)PyMem_Malloc(sizeof(struct iovec) * 2);
 
    iov[0].iov_base = data;
    iov[0].iov_len = data_len;
    total += iov[0].iov_len;

    iov[1].iov_base = "\r\n";
    iov[1].iov_len = 2;
    total += iov[1].iov_len;

    request_send_data(client, env, iov, 2, total, false);

    return 1;

}


static int
request_retrieval(Client *client, PyObject *env, char *key, size_t key_len, char *data, size_t data_len, unsigned short flags, uint64_t cas_unique)
{
    size_t total = 0;
    struct iovec *iov;
    int iov_cnt = 7;
    bool end = 0;
    int index = 0;
    
    if(client->key_num == 1){
        //last
        iov_cnt = 8;
        end = 1;
    }
    if(cas_unique > 0){
        iov_cnt++;
    }
    
    iov = (struct iovec *)PyMem_Malloc(sizeof(struct iovec) * iov_cnt);
    
    //1
    iov[index].iov_base = "VALUE ";
    iov[index].iov_len = 6;
    total += iov[index].iov_len;
    index++;
    
    //2
    iov[index].iov_base = key;
    iov[index].iov_len = key_len;
    total += iov[index].iov_len;
    index++;

    char *flag_str = PyMem_Malloc(sizeof(char) * 32);
    sprintf(flag_str, " %d ", flags);
    
    //3
    iov[index].iov_base = flag_str;
    iov[index].iov_len = strlen(flag_str);
    total += iov[index].iov_len;
    index++;

    char *data_len_str = PyMem_Malloc(sizeof(char) * 32);
    sprintf(data_len_str, "%d ", data_len);
    
    //4
    iov[index].iov_base = data_len_str;
    iov[index].iov_len = strlen(data_len_str);
    total += iov[index].iov_len;
    index++;
    
    if(cas_unique > 0){
        char *cas_unique_str = PyMem_Malloc(sizeof(char) * 64);
        sprintf(cas_unique_str, "%lld ", cas_unique);
        //
        iov[index].iov_base = cas_unique_str;
        iov[index].iov_len = strlen(cas_unique_str);
        total += iov[index].iov_len;
        index++;
    }

    //5
    iov[index].iov_base = "\r\n";
    iov[index].iov_len = 2;
    total += iov[index].iov_len;
    index++;
    
    //6
    iov[index].iov_base = data;
    iov[index].iov_len = data_len;
    total += iov[index].iov_len;
    index++;
    
    // 7
    iov[index].iov_base = "\r\n";
    iov[index].iov_len = 2;
    total += iov[index].iov_len;
    index++;

    if(end){
#ifdef DEBUG
        //printf("END write key addr=%p key=%s, total=%d\n", iov[1].iov_base, iov[1].iov_base, total);
        //printf("END write data addr=%p data=%s, total=%d\n", iov[4].iov_base, iov[4].iov_base, total);
#endif 
        //8
        iov[index].iov_base = "END\r\n";
        iov[index].iov_len = 5;
        total += iov[index].iov_len;
    }else{
#ifdef DEBUG
        //printf("write key addr=%p key=%s, total=%d\n", iov[1].iov_base, iov[1].iov_base, total);
        //printf("write data addr=%p data=%s, total=%d\n", iov[4].iov_base, iov[4].iov_base, total);
#endif 
    }
    request_send_data(client, env, iov, iov_cnt, total, cas_unique > 0);
    client->key_num--;
    return 1;
}

static int
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
    client = PyMem_Malloc(sizeof(client_t)); 
    client->fd = fd;
    client->input_buf = malloc(sizeof(char) * BUFSIZE);
    client->input_buf_size = sizeof(char) * BUFSIZE;
    client->input_pos = 0;
    client->input_len = 0;
    client->remote_addr = remote_addr;
    client->remote_port = remote_port;
    pyclient->client = client;
    if(binary_protocol){
        init_binary_parser(pyclient);
    }else{
        init_text_parser(pyclient);
    }
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
    ret = request_numeric(client, env, data, data_len);
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
    ret = request_simple(client, env, data, data_len);
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
    ret = request_simple(client, env, data, data_len);
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

    ret = request_retrieval(client, env, key, key_len, data, data_len, flags, cas_unique);
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
    client = self->client;
    buf_write(client, buf, read_length);
    if(binary_protocol){
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
    if(binary_protocol){
        init_binary_parser(self);
    }else{
        init_text_parser(self);
    }
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
Client_New(int fd, char *remote_addr, int remote_port)
{
    Client *self;

    self = PyObject_NEW(Client, &ClientType);
    if(self == NULL){
        return NULL;
    }
    self->fd = fd;
    self->key_num = 0;
    self->data = NULL;
    client_t_new(self, fd, remote_addr, remote_port);
    return (PyObject *)self;
}

static void
Client_dealloc(Client* self)
{
    //printf("call dealloc \n");
    self->server = NULL;
    self->client = NULL;
    self->key_num = 0;
    self->data = NULL;
    //self->env = NULL;
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
