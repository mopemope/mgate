#ifndef BINARY_RESPONSE_H
#define BINARY_RESPONSE_H

#include <Python.h>
#include "../client.h"


inline void 
binary_error_response(Client *client, char *msg);

inline int 
write_binary_response(Client *self, PyObject *env, PyObject *response);

#endif
