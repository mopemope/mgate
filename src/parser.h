#ifndef parser_h
#define parser_h

void init_parser(Client *client);
int execute_parse(Client *client, const char* data, size_t len, size_t* off);

#endif
