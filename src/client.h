#ifndef client_h
#define client_h

#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include "memproto/memtext.h"
#include "memproto/memproto.h"


typedef struct iovec iovec_t;

typedef struct {
    int fd;
    char *remote_addr;
    int remote_port;
    char *input_buf;
    size_t input_buf_size;
    size_t input_pos;
    size_t input_len;
    void *parser;
    void *callback;          
    //memtext_parser *parser;
    //memtext_callback *callback;          
} client_t;

typedef struct {
    PyObject_HEAD
    client_t *client;
    PyObject *server;
    int fd;
    int key_num;
    void *data;
    bool binary;
} Client;

typedef struct {
    int fd;
    iovec_t *iov;
    int iov_cnt;
    int total;
    void *next;
    bool cas;
    PyObject *env;
} write_data;


PyObject * 
Client_New(int fd, char *remote_addr, int remote_port);

void 
Client_exec_parse(Client *self, char *buf, size_t read_length);

void
Client_clear(Client *client);

void
Client_close(Client *client);


extern PyTypeObject ClientType;

int 
write_response(Client *client, PyObject *env, PyObject *response);

void 
write_error_response(Client *client, char *msg);

#endif
