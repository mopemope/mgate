#ifndef B_PARSER_H
#define B_PARSER_H

void init_binary_parser(Client *client);
int execute_binray_parse(Client *client, const char* data, size_t len, size_t* off);

#endif
