#ifndef UTIL_H
#define UTIL_H

#include "client.h"

inline void 
setup_listen_sock(int fd);

inline void 
setup_sock(int fd);

inline void 
enable_cork(client_t *client);

inline void 
disable_cork(client_t *client);

#endif
