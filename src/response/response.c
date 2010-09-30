#include "response.h"

inline int
send_writev(write_bucket *data)
{
    size_t w;
    int i = 0;
    w = writev(data->fd, data->iov, data->iov_cnt);
    if(w == -1){
        //error
        if (errno == EAGAIN || errno == EWOULDBLOCK) { 
            /* try again later */
            return 0;
        }else{
            return -1;
        }
    }if(w == 0){
        //is dead
        return -1;
    }else{
        if(data->total > w){
            for(; i < data->iov_cnt;i++){
                if(w > data->iov[i].iov_len){
                    //already write
                    w -= data->iov[i].iov_len;
                    data->iov[i].iov_len = 0;
                }else{
                    data->iov[i].iov_base += w;
                    data->iov[i].iov_len = data->iov[i].iov_len - w;
                    break;
                }
            }
            data->total = data->total -w;
            //resume
            return 0;
        }
    }
    return 1;

}


inline void
request_send_data(Client *client, PyObject *env, struct iovec *iov, int iov_cnt, size_t total, bool cas)
{
    write_bucket *new_bucket;

    new_bucket = PyMem_Malloc(sizeof(write_bucket));
    memset(new_bucket, 0, sizeof(write_bucket));
    new_bucket->env = env;
    new_bucket->next = NULL;
    new_bucket->fd = client->fd;
    new_bucket->iov = iov;
    new_bucket->iov_cnt = iov_cnt;
    new_bucket->total = total;
    new_bucket->cas = cas;
    new_bucket->binary_protocol = client->binary_protocol;

    return send_bucket(client, new_bucket);

}

