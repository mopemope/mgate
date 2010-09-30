#include "../server.h"


static inline int
set_get_env_internal(PyObject *env, char *c_key, size_t c_key_len);

static inline void
set_base_env(PyObject *env, Client *pyclient, memtext_command cmd)
{
    PyObject *key = PyString_FromString("cmd");
    PyObject *object = Py_BuildValue("l", cmd);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("_client");
    PyDict_SetItem(env, key, (PyObject *)pyclient);
    Py_DECREF(key);

}

static inline int
set_numeric_env(PyObject *env, memtext_request_numeric *req)
{
    int ret=0;
	const char* c_key = req->key;
	const size_t c_key_len  = req->key_len;

	uint32_t c_value = req->value;
    bool c_noreply = req->noreply;


    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("value");
    object = Py_BuildValue("I", c_value);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("noreply");
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
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

    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("exptime");
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("noreply");
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
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
    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    ret = PyDict_SetItem(env, key, object);
    Py_DECREF(key);
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

    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("data");
    object = PyString_FromStringAndSize(c_data, c_data_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("exptime");
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("flags");
    object = Py_BuildValue("H", c_flags);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("noreply");
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
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


    PyObject *key = PyString_FromString("key");
    PyObject *object = PyString_FromStringAndSize(c_key, c_key_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("data");
    object = PyString_FromStringAndSize(c_data, c_data_len);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("exptime");
    object = Py_BuildValue("I", c_exptime);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("flags");
    object = Py_BuildValue("H", c_flags);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("noreply");
    object = PyBool_FromLong(c_noreply);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);

    key = PyString_FromString("cas_unique");
    object = Py_BuildValue("k", c_cas_unique);
    PyDict_SetItem(env, key, object);
    Py_DECREF(key);
    Py_DECREF(object);
    
    return ret;
    
}

static inline int
storage(Client *pyclient, memtext_command cmd, memtext_request_storage *req)
{
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    server = pyclient->server;
    
    env = PyDict_New();
    //set env
    set_storage_env(env, req);
    
    switch(cmd){
        case MEMTEXT_CMD_SET:
            method_name = PyString_FromString("set");
            break;
        case MEMTEXT_CMD_ADD:
            method_name = PyString_FromString("add");
            break;
        case MEMTEXT_CMD_REPLACE:
            method_name = PyString_FromString("replace");
            break;
        case MEMTEXT_CMD_APPEND:
            method_name = PyString_FromString("append");
            break;
        case MEMTEXT_CMD_PREPEND:
            method_name = PyString_FromString("prepend");
            break;
        default:
            break;
    }
    if(method_name){
        set_base_env(env, pyclient, cmd);
        proxy = PyDictProxy_New(env);
        response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
        //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
        Py_XDECREF(method_name);
        Py_DECREF(proxy);
    }else{
        goto error;
    }

    if(PyErr_Occurred()){
        //TODO error
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    /*
    if(response == Py_None){
        //lazy
        pyclient->status = CALLED;
        ret = 0;
    }else{
        ret = write_response(pyclient, env, response);
    }*/
    return ret;
error:
    Py_XDECREF(response);
    return -1;

}

static inline int
cas(Client *pyclient, memtext_command cmd, memtext_request_cas *req)
{
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    server = pyclient->server;
    
    env = PyDict_New();
    //set env
    set_cas_env(env, req);
    
    method_name = PyString_FromString("cas");
    set_base_env(env, pyclient, cmd); 
    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
    //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
    Py_XDECREF(method_name);
    Py_DECREF(proxy);

    if(PyErr_Occurred()){
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    /*
    if(response == Py_None){
        //lazy
        pyclient->status = CALLED;
        ret = 0;
    }else{
        ret = write_response(pyclient, env, response);
    }*/
    return ret;
error:
    Py_XDECREF(response);
    return -1;

}

static inline int
delete(Client *pyclient, memtext_command cmd, memtext_request_delete *req)
{
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    server = pyclient->server;
    
    env = PyDict_New();
    //set env
    set_delete_env(env, req);
    
    method_name = PyString_FromString("delete");
    set_base_env(env, pyclient, cmd);
    
    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
    //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
    Py_DECREF(method_name);
    Py_DECREF(proxy);

    if(PyErr_Occurred()){
        //TODO error
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    /*
    if(response == Py_None){
        //lazy
        pyclient->status = CALLED;
        ret = 0;
    }else{
        ret = write_response(pyclient, env, response);
    }*/
    return ret;
error:
    Py_XDECREF(response);
    return -1;

}

static inline int 
get(Client *pyclient, memtext_command cmd, memtext_request_retrieval *req, char *call_method)
{
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    server = pyclient->server;
    env = PyDict_New();

    //set env
    set_get_env(env, req);

    method_name = PyString_FromString(call_method);
    set_base_env(env, pyclient, cmd);
    
    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
    //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
    Py_DECREF(method_name);
    Py_DECREF(proxy);
    
    if(PyErr_Occurred()){
        //TODO error
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    /* 
    if(response == Py_None){
        //lazy
        pyclient->status = CALLED;
        ret = 0;
    }else{
        ret = write_response(pyclient, env, response);
    }*/
    return ret;
error:
    Py_XDECREF(response);
    return -1;

}

static inline int 
get_multi(Client *pyclient, memtext_command cmd, memtext_request_retrieval *req, char *call_method)
{
    PyObject *server, *env, *proxy, *response=NULL, *method_name=NULL;
    int ret = -1;
    unsigned i; 
    char *c_key;
    size_t c_key_len;

    server = pyclient->server;
    
	for(i = 0; i < req->key_num; i++) {
#ifdef DEBUG
        printf("key = %s key_len = %d\n", (char *)req->key[i], req->key_len[i]);
#endif
        env = PyDict_New();
        c_key = (char *)req->key[i];
	    c_key_len  = req->key_len[i];
        set_get_env_internal(env, c_key, c_key_len);
        
        method_name = PyString_FromString(call_method);
        set_base_env(env, pyclient, cmd);

        proxy = PyDictProxy_New(env);
        response = PyObject_CallMethodObjArgs(server, method_name, proxy, NULL);
        //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
        Py_DECREF(method_name);
        Py_DECREF(proxy);

        if(PyErr_Occurred()){
            //TODO error
            PyErr_Print();
            //exit(-1);
            loop_done = 0;
            goto error;
        }

        /*
        if(response == Py_None){
            //lazy
            pyclient->status = CALLED;
            ret = 0;
        }else{
            //direct write response
            ret = write_response(pyclient, env, response);
        }
        */
        Py_XDECREF(response);
    }
    
    return ret;
    
error:
    Py_XDECREF(response);
    return -1;

}

static inline int
numeric(Client *pyclient, memtext_command cmd, memtext_request_numeric *req)
{
    PyObject *server, *proxy, *env, *response=NULL, *method_name=NULL;
    int ret = -1;
    
    //save cmd type
    server = pyclient->server;

    env = PyDict_New();
    //set env
    set_numeric_env(env, req);

    switch(cmd){
        case MEMTEXT_CMD_INCR:
            method_name = PyString_FromString("incr");
            break;
        case MEMTEXT_CMD_DECR:
            method_name = PyString_FromString("decr");
            break;
        default:
            break;
    }
    set_base_env(env, pyclient, cmd);

    proxy = PyDictProxy_New(env);
    response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
    //response = PyObject_CallMethodObjArgs(server, method_name, pyclient, proxy, NULL);
    Py_DECREF(method_name);
    Py_DECREF(proxy);
    
    if(PyErr_Occurred()){
        //TODO error
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    /*
    if(response == Py_None){
        //lazy
        pyclient->status = CALLED;
        ret = 0;
    }else{
        ret = write_response(pyclient, env, response);
    }*/
    return ret;
error:
    Py_XDECREF(response);
    return -1;

}


static inline int 
retrieval_callback(void* user, memtext_command cmd, memtext_request_retrieval* req)
{
    int ret = 0;
    char *call_method;

    Client *client = (Client *)user;
    client->key_num = req->key_num;
#ifdef DEBUG
    printf("call get or gets %d key_num %d\n", client->fd, client->key_num);
#endif
    
    if(cmd == MEMTEXT_CMD_GET){
        call_method = "get";
    }else{
        call_method = "gets";
    }

	if(req->key_num == 1) {
		ret = get(client, cmd, req, call_method);
	} else {
		ret = get_multi(client, cmd, req, call_method);
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




