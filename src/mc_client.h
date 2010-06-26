#ifndef mc_client_h
#define mc_client_h

#include <Python.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "picoev.h"
#include "memclient/text_response.h"

#define MAX_SERVER 128
#define POINTS_PER_SERVER 100
#define MAX_HOST_LENGTH 128

typedef struct iovec iovec_t;

typedef enum {
    READY,
    CONNECTED,
    DEAD,
} client_status;

typedef struct {
    int fd;
    iovec_t *iov;
    int iov_cnt;
    int total;
    void *next;
    //bool cas;
    //PyObject *env;

} request_data;

typedef struct{
    uint32_t index;
    uint32_t value;
} point_item_t;

typedef struct {
    int fd;
    char *hostname;
    int port;
    char *input_buf;
    size_t input_buf_size;
    size_t input_pos;
    size_t input_len;
    mctext_parser *parser;
    //memtext_callback *callback;          
    client_status status;
    struct addrinfo *ai;
    request_data *request_data;
} memserver_t;

typedef struct {
    point_item_t *points;
    memserver_t *server_list;
    uint32_t server_count;
    uint32_t points_count;
} consistent_t;

typedef struct {
    PyObject_HEAD
    //memserver_t mem_servers[MAX_SERVER];
    PyObject *gate_server;
    uint32_t server_count;
} MemClient;

#endif
