#ifndef RESPONSE_H
#define RESPONSE_H

#include <Python.h>
#include "../client.h"

inline int
send_writev(write_bucket *data);

inline void
request_send_data(Client *client, PyObject *env, struct iovec *iov, int iov_cnt, size_t total, bool cas);

#endif
