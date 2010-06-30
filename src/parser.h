#ifndef parser_h
#define parser_h

void init_text_parser(Client *client);
int execute_text_parse(Client *client, const char* data, size_t len, size_t* off);

#endif
