#ifndef CLIENT_H
#define CLIENT_H

#include <Python.h>
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
    uint8_t binary_protocol;
    uint8_t tcp_cork;
} Client;

typedef struct {
    int fd;
    iovec_t *iov;
    int iov_cnt;
    int total;
    void *next;
    bool cas;
    PyObject *env;
    uint8_t binary_protocol;
} write_bucket;


inline PyObject * 
Client_New(PyObject *server, int fd, char *remote_addr, int remote_port);

inline int  
Client_exec_parse(Client *self, char *buf, size_t read_length);

inline void
Client_clear(Client *client);

inline void
Client_close(Client *client);


extern PyTypeObject ClientType;

inline int 
write_response(Client *client, PyObject *env, PyObject *response);

inline void 
write_error_response(Client *client, char *msg);


/*
typedef union {
    struct {
        uint8_t magic;
        uint8_t opcode;
        uint8_t data_type;
        uint16_t reserved;
        uint32_t opaque;
        uint64_t cas;
    } header;
    uint8_t bytes[24];
} response_header;


typedef union {
    struct {
        response_header header;
    } message;
    uint8_t bytes[sizeof(response_header)];
} response;
*/

#endif
