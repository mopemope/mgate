#ifndef text_response_h
#define text_response_h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MEMTEXT_MAX_MULTI_GET 256

typedef struct {
	size_t key_pos;
	char *key;
	unsigned int key_len;

	size_t flags;
	uint32_t exptime;
	size_t bytes;
	bool noreply;
	uint64_t cas_unique;

	size_t data_pos;
	unsigned int data_len;
	char *data;
	size_t data_count;

} mctext_response;


typedef struct {
	size_t data_count;

	int cs;
	int top;
	int stack[1];
    
    mctext_response res[MEMTEXT_MAX_MULTI_GET];
    mctext_response *cur;
    
    unsigned int res_num;

	void* user;
} mctext_parser;

void 
mctext_init(mctext_parser* ctx, void* user);

int 
mctext_execute(mctext_parser* ctx, const char* data, size_t len, size_t* off);


#endif

