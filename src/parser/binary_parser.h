#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H

#include "../client.h"

inline void 
init_binary_parser(Client *client);

inline int 
execute_binray_parse(Client *client, const char* data, size_t len, size_t* off);

#endif

