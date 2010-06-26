#include "text_response.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MARK(M, FPC) (ctx->cur->M = FPC - data)

#define MARK_LEN(M, FPC) (FPC - (ctx->cur->M + data))

#define MARK_PTR(M) (ctx->cur->M + data)

#define NUM_BUF_MAX 20

#define SET_INTEGER(DST, M, FPC, STRFUNC) \
	do { \
		pos = MARK_PTR(M); \
		if(pos[0] == '0') { ctx->res->DST = 0; } \
		else { \
			len = MARK_LEN(M, FPC); \
			if(len > NUM_BUF_MAX) { goto convert_error; } \
			memcpy(numbuf, pos, len); \
			numbuf[len] = '\0'; \
			ctx->cur->DST = STRFUNC(numbuf, NULL, 10); \
			if(ctx->cur->DST == 0) { goto convert_error; } \
		} \
	} while(0)


#define SET_UINT(DST, M, FPC) \
	SET_INTEGER(DST, M, FPC, strtoul)

#define SET_ULL(DST, M, FPC) \
	SET_INTEGER(DST, M, FPC, strtoull)


#define SET_MARK_LEN(DST, M, FPC) \
		ctx->cur->DST = MARK_LEN(M, FPC);

%%{
	machine memc;
    
	action mark_key {
		MARK(key_pos, fpc);
	}
	action key {
        printf("key\n");
		SET_MARK_LEN(key_len, key_pos, fpc);
	}

    action incr_key {
        printf("incr_key\n");
		ctx->cur++;
		//if(ctx->responses > MEMTEXT_MAX_MULTI_GET) {
		//	goto convert_error;
		//}
	}

	action mark_flags {
		MARK(flags, fpc);
	}
	action flags {
		SET_UINT(flags, flags, fpc);
	}

	action mark_exptime {
		MARK(exptime, fpc);
	}
	action exptime {
		SET_UINT(exptime, exptime, fpc);
	}

	action mark_bytes {
		MARK(bytes, fpc);
	}
	action bytes {
		SET_UINT(bytes, bytes, fpc);
	}

	action noreply {
		ctx->cur->noreply = true;
	}

	action mark_cas_unique {
		MARK(cas_unique, fpc);
	}
	
    action cas_unique {
		SET_ULL(cas_unique, cas_unique, fpc);
	}
	
    action mark_data {
        printf("mark_data\n");
		MARK(data_pos, fpc);
        printf("data_pos %d\n", ctx->res->data_pos);
	}
	action simple_data {
        printf("simple_data %s\n", fpc);
		SET_MARK_LEN(data_len, data_pos, fpc);
	}

	action data_start {
        printf("data_start\n");
		MARK(data_pos, fpc+1);
		ctx->cur->data_count = ctx->cur->bytes;
		fcall data;
	}

	action data {
        //printf("data \n");
		//if(--ctx->data_count == 0) {
		if(--ctx->cur->data_count == 0) {
			SET_MARK_LEN(data_len, data_pos, fpc+1);
			fret;
		}
	}

	action do_stored {

        SET_MARK_LEN(data_len, data_pos, fpc+1);
        ctx->cur->data = (char *)MARK_PTR(data_pos);
    }

	action do_retrieval {
        printf("do_retrieval\n");
        ctx->cur->key = (char *)MARK_PTR(key_pos);
        ctx->cur->data = (char *)MARK_PTR(data_pos);
		ctx->cur++;
        ctx->res_num++;
        
        /*
        unsigned int i;
		++ctx->keys;
		for(i=0; i < ctx->keys; ++i) {
			ctx->key_pos[i] = (size_t)MARK_PTR(key_pos[i]);
		}
		CALLBACK(cb, mctext_callback_retrieval);
		if(cb) {
			mctext_request_retrieval req = {
				(const char**)ctx->key_pos,
				ctx->key_len,
				ctx->keys
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
        */
	}
    
    action catch_err{
        SET_MARK_LEN(data_len, data_pos, fpc+1);
        printf("error\n");
    }

	key        = ([^\r \0\n]+)       >mark_key        %key;
	flags      = ('0' | [1-9][0-9]*) >mark_flags      %flags;
	bytes      = ([1-9][0-9]*)       >mark_bytes      %bytes;
	cas_unique = ('0' | [1-9][0-9]*) >mark_cas_unique %cas_unique;
    
    msg = any* -- "\r\n";

    error_helper = "ERROR\r\n"
                 | "CLIENT_ERROR " msg "\r\n"
                 | "SERVER_ERROR " msg "\r\n"
                 ;

    error = error_helper @catch_err;
    
    stored_response = "STORED\r\n"
             | "NOT_STORED\r\n"
             | "EXISTS\r\n"
             | "NOT_FOUND\r\n" 
             ;

    retrieval_response = 'VALUE ' key ' ' flags ' ' bytes
                        (' ' cas_unique)?
                        '\r\n'
                        @data_start
                        '\r\n'
                        ;

	command = error
            | (stored_response >mark_data @do_stored)
            | (retrieval_response @do_retrieval)+
			;

main := (command);
                         
data := (any @data)*;
}%%

%% write data;

void mctext_init(mctext_parser* ctx, void* user)
{
	int cs = 0;
	int top = 0;
	%% write init;
	memset(ctx, 0, sizeof(mctext_parser));
	ctx->cs = cs;
	//ctx->callback = *callback;
	ctx->user = user;
    ctx->cur = &(ctx->res[0]);
}

int mctext_execute(mctext_parser* ctx, const char* data, size_t len, size_t* off)
{

	if(len <= *off) { return 0; }
    
	const char* p = data + *off;
	const char* pe = data + len;
	const char* eof = pe;
	int cs = ctx->cs;
	int top = ctx->top;
	int* stack = ctx->stack;
	const char* pos;
	char numbuf[NUM_BUF_MAX+1];


	%% write exec;

ret:
	ctx->cs = cs;
	ctx->top = top;
	*off = p - data;

	if(cs == memc_error) {
		return -1;
	} else if(cs == memc_first_final) {
		return 1;
	} else {
		return 0;
	}

convert_error:
	cs = memc_error;
	goto ret;
}


