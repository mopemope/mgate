#ifndef TEXT_RESPONSE_H
#define TEXT_RESPONSE_H

#include <Python.h>
#include "../client.h"

inline void 
text_error_response(Client *client, char *msg);

inline int 
text_simple_response(Client *client, PyObject *env, char *data, size_t data_len);

inline int 
text_numeric_response(Client *client, PyObject *env, char *data, size_t data_len);

inline int
text_get_response(Client *client, PyObject *env, char *key, size_t key_len, char *data, size_t data_len, unsigned short flags, uint64_t cas_unique);

#endif
