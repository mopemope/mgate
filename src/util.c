#include "util.h"

#include <unistd.h>
#include <fcntl.h>   
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

inline void 
setup_listen_sock(int fd)
{
    int on = 1, r;
    r = setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &on, sizeof(on));
    assert(r == 0);
    r = fcntl(fd, F_SETFL, O_NONBLOCK);
    assert(r == 0);
}

inline void 
setup_sock(int fd)
{
    int on = 1, r;
    r = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    assert(r == 0);
    r = fcntl(fd, F_SETFL, O_NONBLOCK);
    assert(r == 0);
}

inline void 
enable_cork(int fd)
{
    int on = 1, r;
    r = setsockopt(fd, IPPROTO_TCP, TCP_CORK, &on, sizeof(on));
    assert(r == 0);
}

inline void 
disable_cork(int fd)
{
    int off = 0;
    int on = 1, r;
    r = setsockopt(fd, IPPROTO_TCP, TCP_CORK, &off, sizeof(off));
    assert(r == 0);

    r = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    assert(r == 0);
}
