#include "binary_response.h"

static inline int
binary_get_response(Client *client, PyObject *env, char *key, size_t key_len, char *data, size_t data_len, unsigned short flags, uint64_t cas_unique)
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

    ret = binary_get_response(client, env, key, key_len, data, data_len, flags, cas_unique);
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

static inline int
write_retrieval_response(Client *client, PyObject *env, PyObject *response)
{
    int fd, ret;
    PyObject *pcmd = PyDict_GetItemString(env, "cmd");
    memtext_command cmd = (memtext_command)PyInt_AsLong(pcmd);
    fd = client->fd;

    if(PyTuple_Check(response)){
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

    }else{
        ret = write_retrieval(client, env, response, 0, 0);
    }
    return ret;
}

inline void 
binary_error_response(Client *client, char *msg)
{
}

inline int 
write_binary_response(Client *client, PyObject *env, PyObject *response)
{
    int ret;
    PyObject *pcmd = PyDict_GetItemString(env, "cmd");
    memproto_command cmd = (memproto_command)PyInt_AsLong(pcmd);
    switch(cmd){
        /*retrieval*/
        case MEMPROTO_CMD_GET:
        case MEMPROTO_CMD_GETK:
        case MEMPROTO_CMD_GETQ:
        case MEMPROTO_CMD_GETKQ:
            ret = write_retrieval_response(client, env, response);
            break;
        default:
            ret = -1;

    }
    return ret;
}

