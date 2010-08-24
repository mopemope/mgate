#ifndef SERVER_H
#define SERVER_H

#include <Python.h>

#include <stddef.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <stdbool.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "picoev.h"
#include "client.h"

typedef struct {
    PyObject_HEAD
    char *host;
    int port;
    int listen_fd;
    char *unix_sock_name;
    picoev_loop *main_loop;
} ServerObject;


#define STORED "STORED\r\n"
#define NOT_STORED "NOT_STORED\r\n"
#define EXISTS "EXISTS\r\n"
#define NOT_FOUND "NOT_FOUND\r\n"
#define DELETED "DELETED\r\n"

//void
//Sever_add_writer(Client *client);

//void
//Sever_add_wevent(Client *client, picoev_handler* callback, void* cb_arg);

inline void
request_send_data(Client *client, PyObject *env, struct iovec *iov, int iov_cnt, size_t total, bool cas);

int loop_done;

#endif

