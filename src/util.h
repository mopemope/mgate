#ifndef UTIL_H
#define UTIL_H

#include "client.h"

inline void 
setup_listen_sock(int fd);

inline void 
setup_sock(int fd);

inline void 
enable_cork(int fd);

inline void 
disable_cork(int fd);

#endif
