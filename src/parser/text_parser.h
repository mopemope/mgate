#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include "../client.h"

inline void 
init_text_parser(Client *client);

inline int 
execute_text_parse(Client *client, const char* data, size_t len, size_t* off);



#endif
