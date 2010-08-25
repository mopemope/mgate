#include "../server.h"
#include "text_response.h"

static inline int 
write_retrieval(Client *client, PyObject *env, PyObject *response, unsigned short flags, uint64_t cas_unique);

inline void
text_error_response(Client *client, char *error)
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

static inline int 
text_simple_response(Client *client, PyObject *env, char *data, size_t data_len)
{

    size_t total = 0;
    struct iovec *iov;
    iov = (struct iovec *)PyMem_Malloc(sizeof(struct iovec) * 1);
 
    iov[0].iov_base = data;
    iov[0].iov_len = data_len;
    total += iov[0].iov_len;

#ifdef DEBUG
    printf("text_simple_response response:%s\n", data);
    printf("text_simple_response iov:%p\n", iov);
#endif

    request_send_data(client, env, iov, 1, total, false);

    return 1;
}

static inline int 
text_numeric_response(Client *client, PyObject *env, char *data, size_t data_len)
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


static inline int
text_get_response(Client *client, PyObject *env, char *key, size_t key_len, char *data, size_t data_len, unsigned short flags, uint64_t cas_unique)
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
        //8
        iov[index].iov_base = "END\r\n";
        iov[index].iov_len = 5;
        total += iov[index].iov_len;
    }
    request_send_data(client, env, iov, iov_cnt, total, cas_unique > 0);
    client->key_num--;
    return 1;
}

static inline int 
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

static inline int 
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

static inline int 
write_storage(Client *client, PyObject *env, PyObject *response)
{
    char *data;
    Py_ssize_t data_len;
    int ret;

#ifdef DEBUG
    printf("call write_storage \n");
#endif

    if(PyBool_Check(response)){
        //BOOL
        if(PyObject_IsTrue(response)){
            ret = text_simple_response(client, env, STORED, 8);
        }else{
            ret = text_simple_response(client, env, NOT_STORED, 12);
        }
    }else if(PyString_Check(response)){
        if(PyString_AsStringAndSize(response, &data, &data_len)){
            //TODO raise Error
            goto error;
        }
        ret = text_simple_response(client, env, data, data_len);
    }else{
        //TODO error

        goto error;
    }

    
    if(ret < 0){
        //write_error
        //raise Error
        goto error;
    }
    //Py_DECREF(response);
    return ret;
error:
#ifdef DEBUG
    printf("error \n");
#endif 
    //Py_DECREF(response);
    return 0;

}

static inline int 
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

static inline int 
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

inline int 
write_text_response(Client *self, PyObject *env, PyObject *response)
{
    int ret;
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
