#ifndef BINARY_RESPONSE_H
#define BINARY_RESPONSE_H

#include <Python.h>
#include "../client.h"
#include "response.h"


struct {
    uint8_t magic;
    uint8_t opcode;
    uint8_t data_type;
    uint16_t reserved;
    uint32_t opaque;
    uint64_t cas;
} header;


inline void 
binary_error_response(Client *client, char *msg);

inline int 
write_binary_response(Client *self, PyObject *env, PyObject *response);

#endif
