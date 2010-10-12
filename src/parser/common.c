#include "common.h"



inline void
setup_env_key(void)
{
    /* env key */
    cmd_key = PyString_FromString("cmd");
    cmdi_key = PyString_FromString("cmdi");
    client_key = PyString_FromString("_client");
    key_key = PyString_FromString("key");
    value_key = PyString_FromString("value");
    flags_key = PyString_FromString("flags");
    data_key = PyString_FromString("data");
    exptime_key = PyString_FromString("exptime");
    noreply_key = PyString_FromString("noreply");
    cas_key = PyString_FromString("cas");
    
    /* method_name */
    m_get = PyString_FromString("get");
    m_gets = PyString_FromString("gets");
    m_set = PyString_FromString("set");
    m_append = PyString_FromString("append");
    m_add = PyString_FromString("add");
    m_replace = PyString_FromString("replace");
    m_prepend = PyString_FromString("prepend");
    m_delete = PyString_FromString("delete");
    m_incr = PyString_FromString("incr");
    m_decr = PyString_FromString("decr");
    m_cas = PyString_FromString("cas");

}

inline void
clear_env_key(void)
{

    /* env key */
    Py_DECREF(cmd_key);
    Py_DECREF(cmdi_key);
    Py_DECREF(client_key);
    Py_DECREF(key_key);
    Py_DECREF(value_key);
    Py_DECREF(flags_key);
    Py_DECREF(data_key);
    Py_DECREF(exptime_key);
    Py_DECREF(noreply_key);
    Py_DECREF(cas_key);
    
    /* method_name */
    Py_DECREF(m_get);
    Py_DECREF(m_gets);
    Py_DECREF(m_set);
    Py_DECREF(m_append);
    Py_DECREF(m_add);
    Py_DECREF(m_replace);
    Py_DECREF(m_prepend);
    Py_DECREF(m_delete);
    Py_DECREF(m_incr);
    Py_DECREF(m_decr);
    Py_DECREF(m_cas);
}


inline int 
call_app(PyObject *pyclient, PyObject *env)
{
    PyObject *args = NULL, PyObject *res = NULL;
    int ret;

    args = Py_BuildValue("(O)", env);
    res = PyObject_CallObject(mgate_app, args, NULL);

    Py_DECREF(args);

    if(res && res == Py_None){
        PyErr_SetString(PyExc_Exception, "response is None");
    }
    if (PyErr_Occurred()){ 
        //TODO Error log
        return NULL;
    }
    
    ret = write_response((Client *)pyclient, env, response);
    
    if(PyErr_Occurred()){
        //TODO error
        PyErr_Print();
        //exit(-1);
        loop_done = 0;
        goto error;
    }
    if(ret < 0){
        //TODO Error

    }
    Py_XDECREF(response);
    return ret;
}

