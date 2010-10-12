#include "../server.h"
#include "binary_parser.h"
#include "common.h"

static inline int
set_base_env(PyObject *env, PyObject *method_name, Client *pyclient, memproto_header *h)
{
    PyObject *object = Py_BuildValue("l", h->opcode);
    
    PyDict_SetItem(env, cmd_key, method_name);
    PyDict_SetItem(env, cmdi_key, object);
    Py_DECREF(object);
    PyDict_SetItem(env, client_key, (PyObject *)pyclient);

    //keep header
    pyclient->header = h;

    return 1;
}

static inline int
set_get_env(PyObject *env, const char *c_key, uint16_t c_key_len)
{
    int ret = 0;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    ret = PyDict_SetItem(env, key_key, object);
    Py_DECREF(object);
    return ret;
}

static inline int
set_storage_env(PyObject *env, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen, uint32_t flags, uint32_t expiration)
{
    int ret = 0;

    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, key_key, object);
    Py_DECREF(object);

    object = PyString_FromStringAndSize(val, vallen);
    PyDict_SetItem(env, data_key, object);
    Py_DECREF(object);

    object = Py_BuildValue("I", expiration);
    PyDict_SetItem(env, exptime_key, object);
    Py_DECREF(object);

    object = Py_BuildValue("H", flags);
    PyDict_SetItem(env, flags_key, object);
    Py_DECREF(object);

    return ret;
    
}

static inline int
set_append_env(PyObject *env, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen)
{
    int ret = 0;

    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, key_key, object);
    Py_DECREF(object);

    object = PyString_FromStringAndSize(val, vallen);
    PyDict_SetItem(env, data_key, object);
    Py_DECREF(object);

    return ret;
}

static inline int
set_delete_env(PyObject *env, const char* key, uint16_t keylen, uint32_t expiration)
{
    int ret = 0;

    PyObject *object = PyString_FromStringAndSize(key, keylen);
    PyDict_SetItem(env, key_key, object);
    Py_DECREF(object);

    object = Py_BuildValue("I", expiration);
    PyDict_SetItem(env, exptime_key, object);
    Py_DECREF(object);
    return ret;
}

static inline void 
cb_get(void* user, memproto_header* h, const char* key, uint16_t keylen)
{

    Client *pyclient;
    PyObject *env;
    int ret = -1;
    
    pyclient = (Client *)user;
    env = PyDict_New();

    //set env
    ret = set_get_env(env, key, keylen);
    set_base_env(env, m_get, pyclient, h);
    call_app(pyclient, env);    

}

static inline void 
cb_storage(void* user, memproto_header* h, const char* key, uint16_t keylen, 
        const char* val, uint32_t vallen, uint32_t flags, uint32_t expiration){
    
    Client *pyclient;
    PyObject *env;
    int ret = -1;
    
    pyclient = (Client *)user;
    env = PyDict_New();

    //set env
    set_storage_env(env, key, keylen, val, vallen, flags, expiration);
    
    switch(h->opcode){
        case MEMPROTO_CMD_SET:
            set_base_env(env, m_set, pyclient, h);
            break;
        case MEMPROTO_CMD_ADD:
            set_base_env(env, m_add, pyclient, h);
            break;
        case MEMPROTO_CMD_REPLACE:
            set_base_env(env, m_replace, pyclient, h);
            break;
        default:
            //TODO ERROR
            break;
    }

    call_app(pyclient, env);    
    
}

static inline void
cb_append(void* user, memproto_header* h, const char* key, uint16_t keylen,
			const char* val, uint32_t vallen){

    Client *pyclient;
    PyObject *env;
    int ret = -1;
    
    pyclient = (Client *)user;
    env = PyDict_New();

    //set env
    set_append_env(env, key, keylen, val, vallen);
    
    switch(h->opcode){
        case MEMPROTO_CMD_APPEND:
            set_base_env(env, m_append, pyclient, h);
            break;
        case MEMPROTO_CMD_PREPEND:
            set_base_env(env, m_prepend, pyclient, h);
            break;
        default:
            //TODO ERROR
            break;
    }

    call_app(pyclient, env);    
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


