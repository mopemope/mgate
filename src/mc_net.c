#include "mc_client.h"

#include <fcntl.h>   
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFSIZE 4096

static void
read_response_callback(picoev_loop* loop, int fd, int events, void* cb_arg);

static int
buf_write(memserver_t *server, const char *c, size_t  l) {
    size_t newl;
    char *newbuf;
    newl = server->input_len + l;
    if (newl >= server->input_buf_size) {
        server->input_buf_size *= 2;
        if (server->input_buf_size <= newl) {
            //assert(newl + 1 < INT_MAX);
            server->input_buf_size = (int)(newl + 1);
        }
        newbuf = (char*)realloc(server->input_buf, server->input_buf_size);
        if (!newbuf) {
            printf("out of memory\n");
            PyErr_SetString(PyExc_MemoryError,"out of memory");
            free(server->input_buf);
            server->input_buf = 0;
            server->input_buf_size = server->input_len = 0;
            return -1;
        }
        server->input_buf = newbuf;
    }

    memcpy(server->input_buf + server->input_len, c , l);

    server->input_len += (int)l;
    return (int)l;
}

request_data *
new_request_data(void){
    request_data *data = PyMem_Malloc(sizeof(request_data));
    memset(data, 0, sizeof(data));
    return data;
}

static void
clear_request_data(request_data *data)
{
    if(data && data->iov){
        /*
        if(data->iov_cnt > 6){
            PyMem_Free(data->iov[2].iov_base);
            PyMem_Free(data->iov[3].iov_base);
        }
        if(data->cas){
            PyMem_Free(data->iov[4].iov_base);
        }*/
        PyMem_Free(data->iov);
        data->iov = NULL;
    }
    data->next = NULL;
    if(data){
        PyMem_Free(data);
        data = NULL;
    }
}

static 
void setup_sock(int fd)
{
    int on = 1, r;

    r = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    assert(r == 0);

    r = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                      &on, (socklen_t)sizeof(int));
    assert(r == 0);

    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    assert(flags != -1);
    
    if ((flags & O_NONBLOCK) == 0){
        r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        assert(r == 0);
    }
}

int  
set_hostinfo(char *hostname, uint32_t port, struct addrinfo *ai)
{
    struct addrinfo hints;
    char str_port[5];
    uint32_t counter= 5;
    int ret;
    snprintf(str_port, 5, "%u", port);
    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype= SOCK_STREAM;
    hints.ai_protocol= IPPROTO_TCP;

    while (--counter) {
        ret = getaddrinfo(hostname, str_port, &hints, &ai);
        if (ret == 0){
            break;
        }else if (ret == EAI_AGAIN){
            struct timespec dream, rem;
            dream.tv_nsec= 1000;
            dream.tv_sec= 0;
            nanosleep(&dream, &rem);
            continue;
        } else {
            //error
            return -1;
        }
    }
    return 1;
}

int 
connect_server(memserver_t *server)
{
    struct addrinfo *ai;
    ai = server->ai;
    if ((server->fd= socket(ai->ai_family,
                           ai->ai_socktype,
                           ai->ai_protocol)) < 0){
        //TODO error
        return -1;
    }

    return 1;
}


static int
send_request(request_data *data)
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
        return 1;
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

static void
connect_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    memserver_t *server = (memserver_t *)cb_arg;
    struct addrinfo *ai = server->ai;
    request_data *data;
    int r = 0;

    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
        picoev_del(loop, fd);
    
    } else if ((events & PICOEV_WRITE) != 0) {
        picoev_set_timeout(loop, fd, 1);
        client_status st = server->status;
        
        switch(st){
            case READY :
                r = connect_server(server);
                if ((connect(server->fd, ai->ai_addr, ai->ai_addrlen) == -1)){
                    //TODO ERROR
                    server->status = DEAD;
                }
                break;
            case CONNECTED:
                // do request
                data = server->request_data;
                if(data){
                    r = send_request(data);
                    if(r < 0){
                        //TODO error
                        server->status = DEAD;
                    }else if(r > 0){
                        //send all data
                        if(data->next){
                            request_data *next = data->next;
                            data->next = NULL;
                            clear_request_data(data);
                            server->request_data = next;
                        }else{
                            server->request_data = NULL;
                            clear_request_data(data);
                        }

                        picoev_del(loop, fd);
                        picoev_add(loop, fd, PICOEV_READ, 1, read_response_callback, server);
                    }
                }
                break;

            case DEAD:
                //close
                break;
            default:
                break;

        }
    }
}

static void
read_response_callback(picoev_loop* loop, int fd, int events, void* cb_arg)
{
    memserver_t *server = (memserver_t *)cb_arg;
    int r = 0;

    if ((events & PICOEV_TIMEOUT) != 0) {

        //timeout
        picoev_del(loop, fd);
    
    } else if ((events & PICOEV_READ) != 0) {

        /* update timeout, and read */
        picoev_set_timeout(loop, fd, 1);
        char buf[MAX_BUFSIZE];
        ssize_t r;
        r = read(fd, buf, sizeof(buf));
        switch (r) {
            case 0: 
                picoev_del(loop, fd);

                break;
            case -1: 
                /* error */
                if (errno == EAGAIN || errno == EWOULDBLOCK) { 
                    /* try again later */
	                break;
                } else { 
                    /* fatal error */
                    picoev_del(loop, fd);
                }
                break;
            default:

                break;
        }

    }

}
