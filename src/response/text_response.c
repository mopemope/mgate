#include "../server.h"
#include "text_response.h"


static inline void
text_error(Client *client, char *error)
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

inline void 
text_error_response(Client *client, char *msg)
{
    text_error(client, msg);
}

inline int 
text_simple_response(Client *client, PyObject *env, char *data, size_t data_len)
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

inline int 
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


inline int
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

