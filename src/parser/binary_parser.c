#include "../server.h"

static inline int
set_base_env(PyObject *env, Client *pyclient, memproto_header *h)
{
    PyObject *key = PyString_FromString("cmd");
    PyObject *object = Py_BuildValue("l", h->opcode);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("_client");
    PyDict_SetItem(env, key, (PyObject *)pyclient);
    Py_DECREF(key);

    //keep header
    pyclient->header = h;

    return 1;
}

static inline int
set_get_env(PyObject *env, const char *c_key, uint16_t c_key_len)
{
    int ret = 0;
    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    ret = PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);
    return ret;
}

static inline int
set_storage_env(PyObject *env, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen, uint32_t flags, uint32_t expiration)
{
    int ret = 0;

    PyObject *pykey = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    pykey = PyString_FromString("data");
    object = PyString_FromStringAndSize(val, vallen);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    pykey = PyString_FromString("exptime");
    object = Py_BuildValue("I", expiration);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    pykey = PyString_FromString("flags");
    object = Py_BuildValue("H", flags);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    return ret;
    
}

static inline int
set_append_env(PyObject *env, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen)
{
    int ret = 0;

    PyObject *pykey = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    pykey = PyString_FromString("data");
    object = PyString_FromStringAndSize(val, vallen);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);


    return ret;
    
}

static inline int
set_delete_env(PyObject *env, const char* key, uint16_t keylen, uint32_t expiration)
{
    int ret = 0;

    PyObject *pykey = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);


    pykey = PyString_FromString("exptime");
    object = Py_BuildValue("I", expiration);
    PyDict_SetItem(env, pykey, object);
    Py_DECREF(pykey);
    Py_DECREF(object);

    return ret;
}

static inline void 
cb_get(void* user, memproto_header* h, const char* key, uint16_t keylen)
{

#ifdef DEBUG
    printf("cb_get\n");
#endif
    Client *pyclient;
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    pyclient = (Client *)user;
    server = pyclient->server;
    env = PyDict_New();

    //set env
    ret = set_get_env(env, key, keylen);

    method_name = PyString_FromString("get");
    set_base_env(env, pyclient, h);
    
    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
    Py_DECREF(method_name);
    Py_DECREF(proxy);
    
    if(PyErr_Occurred()){
        PyErr_Print();
        loop_done = 0;
        goto error;
    }
error:
    Py_XDECREF(response);

}

static inline void 
cb_storage(void* user, memproto_header* h, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen, uint32_t flags, uint32_t expiration){
#ifdef DEBUG
    printf("cb_set\n");
#endif
    Client *pyclient;
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    pyclient = (Client *)user;
    server = pyclient->server;
    env = PyDict_New();

    //set env
    ret = set_storage_env(env, key, keylen, val, vallen, flags, expiration);
    
    //TODO 
    switch(h->opcode){
        case MEMPROTO_CMD_SET:
            method_name = PyString_FromString("set");
            break;
        case MEMPROTO_CMD_ADD:
            method_name = PyString_FromString("add");
            break;
        case MEMTEXT_CMD_REPLACE:
            method_name = PyString_FromString("replace");
            break;
        default:

            break;
    }

    if(!method_name){
        //error
        goto error;
    }

    set_base_env(env, pyclient, h);
    
    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
    Py_DECREF(method_name);
    Py_DECREF(proxy);
    
    if(PyErr_Occurred()){
        PyErr_Print();
        loop_done = 0;
        goto error;
    }
error:
    Py_XDECREF(method_name);
    Py_XDECREF(response);
    
}

static inline void
cb_append(void* user, memproto_header* h, const char* key, uint16_t keylen,
			const char* val, uint32_t vallen){
}

static inline void 
cb_delete(void* user, memproto_header* h, const char* key, uint16_t keylen,
			uint32_t expiration){
}	

static inline void 
cb_numeric(void* user, memproto_header* h,
			const char* key, uint16_t keylen,
			uint64_t amount, uint64_t initial, uint32_t expiration){

}

inline void 
init_binary_parser(Client *pyclient)
{     
    client_t *client = pyclient->client;
    memproto_callback *callback; 
    callback = (memproto_callback *)PyMem_Malloc(sizeof(memproto_callback)); 
    memset(callback, 0, sizeof(memproto_callback));

    callback->cb_get = cb_get; 
    callback->cb_getk = cb_get;
    callback->cb_getq = cb_get;
    callback->cb_getkq = cb_get;
    callback->cb_set = cb_storage;
    callback->cb_add = cb_storage;
    callback->cb_replace = cb_storage;
    
    callback->cb_append = cb_append;
    callback->cb_prepend = cb_append;
    
    callback->cb_delete = cb_delete;
    
    callback->cb_increment = cb_numeric;
    callback->cb_decrement = cb_numeric;

    client->parser = (memproto_parser *)PyMem_Malloc(sizeof(memproto_parser));
    memset(client->parser, 0, sizeof(memproto_parser));
    client->callback = callback;

    memproto_parser_init(client->parser, client->callback, pyclient);
#ifdef DEBUG
    printf("init_binary_parser\n");
#endif

}

inline int 
execute_binray_parse(Client *pyclient, const char* data, size_t len, size_t* off)
{
    int ret = 0;
    client_t *client = pyclient->client;
    memproto_parser *parser = client->parser;
    ret = memproto_parser_execute(parser, data, len, off);
    
    if(ret > 0){
        memproto_dispatch(parser);
    }
#ifdef DEBUG
    printf("execute_binray_parse ret = %d\n", ret);
#endif
    return 0;
}


