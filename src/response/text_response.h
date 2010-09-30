#ifndef TEXT_RESPONSE_H
#define TEXT_RESPONSE_H

#include <Python.h>
#include "../client.h"
#include "response.h"


inline void 
text_error_response(Client *client, char *msg);

inline int 
write_text_response(Client *self, PyObject *env, PyObject *response);

#endif
