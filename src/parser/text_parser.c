#include "../server.h"
#include "text_parser.h"
#include "common.h"

static inline int
set_get_env_internal(PyObject *env, char *c_key, size_t c_key_len);

static inline void
set_base_env(PyObject *env, PyObject *method_name, Client *pyclient, memtext_command cmd)
{
    PyObject *object = Py_BuildValue("l", cmd);
    
    PyDict_SetItem(env, cmd_key, method_name);
    PyDict_SetItem(env, cmdi_key, object);
    Py_DECREF(object);

    PyDict_SetItem(env, client_key, (PyObject *)pyclient);

}


static inline int
set_numeric_env(PyObject *env, memtext_request_numeric *req)
{
    int ret=0;
	const char* c_key = req->key;
	const size_t c_key_len  = req->key_len;

	uint32_t c_value = req->value;
    bool c_noreply = req->noreply;


    PyObject *key = key_key;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = value_key;
    object = Py_BuildValue("I", c_value);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = noreply_key;
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    return ret;
}

static inline int
set_delete_env(PyObject *env, memtext_request_delete *req)
{
    int ret = 0;
	const char* c_key = req->key;
	const size_t c_key_len  = req->key_len;

	uint32_t c_exptime = req->exptime;
    bool c_noreply = req->noreply;

    PyObject *key = key_key;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = exptime_key;
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = noreply_key;
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    return ret;
}

static inline int
set_get_env(PyObject *env, memtext_request_retrieval *req)
{
    int ret = 0;

    char *c_key = (char *)req->key[0];
	size_t c_key_len  = req->key_len[0];
    set_get_env_internal(env, c_key, c_key_len);
    return ret;
}

static inline int
set_get_env_internal(PyObject *env, char *c_key, size_t c_key_len)
{
    int ret = 0;
    PyObject *key = key_key;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    ret = PyDict_SetItem(env, key, object);
    Py_DECREF(object);
    return ret;
}

static inline int
set_storage_env(PyObject *env, memtext_request_storage *req)
{
    int ret =0;

	const char* c_key = req->key;
	const size_t c_key_len  = req->key_len;

	const char* c_data = req->data;
	const size_t c_data_len  = req->data_len;

    unsigned short c_flags = req->flags;
	uint32_t c_exptime = req->exptime;
    bool c_noreply = req->noreply;

    PyObject *key = key_key;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = data_key;
    object = PyString_FromStringAndSize(c_data, c_data_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = exptime_key;
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = flags_key;
    object = Py_BuildValue("H", c_flags);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = noreply_key;;
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);
    return ret;
    
}

static inline int
set_cas_env(PyObject *env, memtext_request_cas *req)
{
    int ret =0;

	const char* c_key = req->key;
	const size_t c_key_len  = req->key_len;

	const char* c_data = req->data;
	const size_t c_data_len  = req->data_len;

    unsigned short c_flags = req->flags;
	uint32_t c_exptime = req->exptime;
	bool c_noreply = req->noreply;
    uint64_t c_cas_unique = req->cas_unique;


    PyObject *key = key_key;
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = data_key;
    object = PyString_FromStringAndSize(c_data, c_data_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = exptime_key;
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = flags_key;
    object = Py_BuildValue("H", c_flags);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = noreply_key;
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);

    key = cas_key;
    object = Py_BuildValue("k", c_cas_unique);
    PyDict_SetItem(env, key, object);
    Py_DECREF(object);
    
    return ret;
    
}

static inline int
storage(Client *pyclient, memtext_command cmd, memtext_request_storage *req)
{
    PyObject *env;
    
    env = PyDict_New();
    //set env
    set_storage_env(env, req);
    
    switch(cmd){
        case MEMTEXT_CMD_SET:
            set_base_env(env, m_set, pyclient, cmd);
            break;
        case MEMTEXT_CMD_ADD:
            set_base_env(env, m_add, pyclient, cmd);
            break;
        case MEMTEXT_CMD_REPLACE:
            set_base_env(env, m_replace, pyclient, cmd);
            break;
        case MEMTEXT_CMD_APPEND:
            set_base_env(env, m_append, pyclient, cmd);
            break;
        case MEMTEXT_CMD_PREPEND:
            set_base_env(env, m_prepend, pyclient, cmd);
            break;
        default:
            break;
    }
    return call_app(pyclient, env);
}

static inline int
cas(Client *pyclient, memtext_command cmd, memtext_request_cas *req)
{
    PyObject *env;
    
    env = PyDict_New();
    //set env
    set_cas_env(env, req);
    set_base_env(env, m_cas, pyclient, cmd); 
    return call_app(pyclient, env);
}

static inline int
delete(Client *pyclient, memtext_command cmd, memtext_request_delete *req)
{
    PyObject *env;
    
    
    env = PyDict_New();
    //set env
    set_delete_env(env, req);
    set_base_env(env, m_delete, pyclient, cmd);
    return call_app(pyclient, env);
    
}

static inline int 
get(Client *pyclient, memtext_command cmd, memtext_request_retrieval *req)
{
    PyObject *env;
    
    env = PyDict_New();

    //set env
    set_get_env(env, req);
    set_base_env(env, m_get, pyclient, cmd);
    return call_app(pyclient, env);
}

static inline int 
get_multi(Client *pyclient, memtext_command cmd, memtext_request_retrieval *req)
{
    PyObject *env;
    int ret = -1;
    unsigned i; 
    char *c_key;
    size_t c_key_len;
    
	for(i = 0; i < req->key_num; i++) {
#ifdef DEBUG
        printf("key = %s key_len = %d\n", (char *)req->key[i], req->key_len[i]);
#endif
        env = PyDict_New();
        c_key = (char *)req->key[i];
	    c_key_len  = req->key_len[i];
        set_get_env_internal(env, c_key, c_key_len);
        
        set_base_env(env, m_get, pyclient, cmd);
        call_app(pyclient, env);
    }
    
    return ret;

}

static inline int
numeric(Client *pyclient, memtext_command cmd, memtext_request_numeric *req)
{
    PyObject *env;

    env = PyDict_New();
    //set env
    set_numeric_env(env, req);

    switch(cmd){
        case MEMTEXT_CMD_INCR:
            set_base_env(env, m_incr, pyclient, cmd);
            break;
        case MEMTEXT_CMD_DECR:
            set_base_env(env, m_decr, pyclient, cmd);
            break;
        default:
            break;
    }
    return call_app(pyclient, env);


}


static inline int 
retrieval_callback(void* user, memtext_command cmd, memtext_request_retrieval* req)
{
    int ret = 0;

    Client *client = (Client *)user;
    client->key_num = req->key_num;
#ifdef DEBUG
    printf("call get or gets %d key_num %d\n", client->fd, client->key_num);
#endif
    

	if(req->key_num == 1) {
		ret = get(client, cmd, req);
	} else {
		ret = get_multi(client, cmd, req);
	}
    return ret;
}

static inline int 
storage_callback(void* user, memtext_command cmd, memtext_request_storage* req)
{
    int ret = 0;
    Client *client = (Client *)user;

    ret = storage(client, cmd, req);
#ifdef DEBUG
    printf("call storage %d\n", client->fd);
#endif
    return ret;
}

static inline int 
cas_callback(void* user, memtext_command cmd, memtext_request_cas* req)
{

    int ret = 0;
    Client *client = (Client *)user;

    ret = cas(client, cmd, req);
#ifdef DEBUG
    printf("call storage %d\n", client->fd);
#endif
    return ret;
    return 0;
}

static inline int 
delete_callback(void* user, memtext_command cmd, memtext_request_delete* req)
{

    int ret = 0;
    Client *client = (Client *)user;

    ret = delete(client, cmd, req);
    return ret;
}


static inline int 
numeric_callback(void* user, memtext_command cmd, memtext_request_numeric* req)
{
    int ret = 0;
    Client *client = (Client *)user;

    ret = numeric(client, cmd, req);
    return ret;
}

inline void 
init_text_parser(Client *pyclient)
{     
    client_t *client = pyclient->client;
    memtext_callback *callback;    
    callback = (memtext_callback *)PyMem_Malloc(sizeof(memtext_callback)); 
    memset(callback, 0, sizeof(memtext_callback));
    callback->cmd_get = retrieval_callback; 
    callback->cmd_gets = retrieval_callback; 
    callback->cmd_set = storage_callback;
    callback->cmd_add = storage_callback;
    callback->cmd_replace = storage_callback;
    callback->cmd_append = storage_callback;
    callback->cmd_prepend = storage_callback;
    callback->cmd_cas = cas_callback;
    callback->cmd_delete = delete_callback;
    callback->cmd_incr = numeric_callback;
    callback->cmd_decr = numeric_callback;

    client->parser = (memtext_parser *)PyMem_Malloc(sizeof(memtext_parser));
    memset(client->parser, 0, sizeof(memtext_parser));
    client->callback = callback;

    memtext_init(client->parser, client->callback, pyclient);
    
}

inline int 
execute_text_parse(Client *pyclient, const char* data, size_t len, size_t* off)
{
    client_t *client = pyclient->client;
    memtext_parser *parser = client->parser;
    memtext_execute(parser, data, len, off);
    return 0;
}




