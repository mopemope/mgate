#line 1 "memtext.rl"
/*
 * memtext
 *
 * Copyright (C) 2008 FURUHASHI Sadayuki
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "memtext.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MARK(M, FPC) (ctx->M = FPC - data)
#define MARK_LEN(M, FPC) (FPC - (ctx->M + data))
#define MARK_PTR(M) (ctx->M + data)

#define NUM_BUF_MAX 20

#define SET_INTEGER(DST, M, FPC, STRFUNC) \
	do { \
		pos = MARK_PTR(M); \
		if(pos[0] == '0') { ctx->DST = 0; } \
		else { \
			len = MARK_LEN(M, FPC); \
			if(len > NUM_BUF_MAX) { goto convert_error; } \
			memcpy(numbuf, pos, len); \
			numbuf[len] = '\0'; \
			ctx->DST = STRFUNC(numbuf, NULL, 10); \
			if(ctx->DST == 0) { goto convert_error; } \
		} \
	} while(0)

#define SET_UINT(DST, M, FPC) \
	SET_INTEGER(DST, M, FPC, strtoul)

#define SET_ULL(DST, M, FPC) \
	SET_INTEGER(DST, M, FPC, strtoull)

#define SET_MARK_LEN(DST, M, FPC) \
		ctx->DST = MARK_LEN(M, FPC);

#define CALLBACK(NAME, TYPE) \
	TYPE NAME = ((TYPE*)(&ctx->callback))[ctx->command]

#line 307 "memtext.rl"




#line 63 "memtext.c"
static const int memtext_start = 1;
static const int memtext_first_final = 129;
static const int memtext_error = 0;

static const int memtext_en_main = 1;
static const int memtext_en_data = 130;

#line 311 "memtext.rl"

void memtext_init(memtext_parser* ctx, memtext_callback* callback, void* user)
{
	int cs = 0;
	int top = 0;
	
#line 78 "memtext.c"
	{
	cs = memtext_start;
	top = 0;
	}
#line 317 "memtext.rl"
	memset(ctx, 0, sizeof(memtext_parser));
	ctx->cs = cs;
	ctx->callback = *callback;
	ctx->user = user;
}

int memtext_execute(memtext_parser* ctx, const char* data, size_t len, size_t* off)
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

	//printf("execute, len:%lu, off:%lu\n", len, *off);
	//printf("%s\n", data);
	//printf("data: ");
	//int i;
	//for(i=0; i < len; ++i) {
	//	printf("0x%x ", (int)data[i]);
	//}
	//printf("\n");

	
#line 113 "memtext.c"
	{
	if ( p == pe )
		goto _test_eof;
	goto _resume;

_again:
	switch ( cs ) {
		case 1: goto st1;
		case 0: goto st0;
		case 2: goto st2;
		case 3: goto st3;
		case 4: goto st4;
		case 5: goto st5;
		case 6: goto st6;
		case 7: goto st7;
		case 8: goto st8;
		case 9: goto st9;
		case 10: goto st10;
		case 11: goto st11;
		case 12: goto st12;
		case 13: goto st13;
		case 14: goto st14;
		case 15: goto st15;
		case 129: goto st129;
		case 16: goto st16;
		case 17: goto st17;
		case 18: goto st18;
		case 19: goto st19;
		case 20: goto st20;
		case 21: goto st21;
		case 22: goto st22;
		case 23: goto st23;
		case 24: goto st24;
		case 25: goto st25;
		case 26: goto st26;
		case 27: goto st27;
		case 28: goto st28;
		case 29: goto st29;
		case 30: goto st30;
		case 31: goto st31;
		case 32: goto st32;
		case 33: goto st33;
		case 34: goto st34;
		case 35: goto st35;
		case 36: goto st36;
		case 37: goto st37;
		case 38: goto st38;
		case 39: goto st39;
		case 40: goto st40;
		case 41: goto st41;
		case 42: goto st42;
		case 43: goto st43;
		case 44: goto st44;
		case 45: goto st45;
		case 46: goto st46;
		case 47: goto st47;
		case 48: goto st48;
		case 49: goto st49;
		case 50: goto st50;
		case 51: goto st51;
		case 52: goto st52;
		case 53: goto st53;
		case 54: goto st54;
		case 55: goto st55;
		case 56: goto st56;
		case 57: goto st57;
		case 58: goto st58;
		case 59: goto st59;
		case 60: goto st60;
		case 61: goto st61;
		case 62: goto st62;
		case 63: goto st63;
		case 64: goto st64;
		case 65: goto st65;
		case 66: goto st66;
		case 67: goto st67;
		case 68: goto st68;
		case 69: goto st69;
		case 70: goto st70;
		case 71: goto st71;
		case 72: goto st72;
		case 73: goto st73;
		case 74: goto st74;
		case 75: goto st75;
		case 76: goto st76;
		case 77: goto st77;
		case 78: goto st78;
		case 79: goto st79;
		case 80: goto st80;
		case 81: goto st81;
		case 82: goto st82;
		case 83: goto st83;
		case 84: goto st84;
		case 85: goto st85;
		case 86: goto st86;
		case 87: goto st87;
		case 88: goto st88;
		case 89: goto st89;
		case 90: goto st90;
		case 91: goto st91;
		case 92: goto st92;
		case 93: goto st93;
		case 94: goto st94;
		case 95: goto st95;
		case 96: goto st96;
		case 97: goto st97;
		case 98: goto st98;
		case 99: goto st99;
		case 100: goto st100;
		case 101: goto st101;
		case 102: goto st102;
		case 103: goto st103;
		case 104: goto st104;
		case 105: goto st105;
		case 106: goto st106;
		case 107: goto st107;
		case 108: goto st108;
		case 109: goto st109;
		case 110: goto st110;
		case 111: goto st111;
		case 112: goto st112;
		case 113: goto st113;
		case 114: goto st114;
		case 115: goto st115;
		case 116: goto st116;
		case 117: goto st117;
		case 118: goto st118;
		case 119: goto st119;
		case 120: goto st120;
		case 121: goto st121;
		case 122: goto st122;
		case 123: goto st123;
		case 124: goto st124;
		case 125: goto st125;
		case 126: goto st126;
		case 127: goto st127;
		case 128: goto st128;
		case 130: goto st130;
	default: break;
	}

	if ( ++p == pe )
		goto _test_eof;
_resume:
	switch ( cs )
	{
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
	switch( (*p) ) {
		case 97: goto tr0;
		case 99: goto tr2;
		case 100: goto tr3;
		case 103: goto tr4;
		case 105: goto tr5;
		case 112: goto tr6;
		case 114: goto tr7;
		case 115: goto tr8;
		case 118: goto tr9;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 291 "memtext.c"
	switch( (*p) ) {
		case 100: goto st3;
		case 112: goto st125;
	}
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	if ( (*p) == 100 )
		goto tr12;
	goto st0;
tr12:
#line 129 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_ADD;     }
	goto st4;
tr130:
#line 132 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_PREPEND; }
	goto st4;
tr136:
#line 130 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_REPLACE; }
	goto st4;
tr138:
#line 128 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_SET;     }
	goto st4;
tr163:
#line 131 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_APPEND;  }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 328 "memtext.c"
	if ( (*p) == 32 )
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto st0;
	}
	goto tr14;
tr14:
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 353 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto tr16;
	}
	goto st6;
tr16:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 371 "memtext.c"
	if ( (*p) == 48 )
		goto tr17;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr18;
	goto st0;
tr17:
#line 78 "memtext.rl"
	{
		MARK(flags, p);
	}
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 387 "memtext.c"
	if ( (*p) == 32 )
		goto tr19;
	goto st0;
tr19:
#line 81 "memtext.rl"
	{
		SET_UINT(flags, flags, p);
	}
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 401 "memtext.c"
	if ( (*p) == 48 )
		goto tr20;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr21;
	goto st0;
tr20:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 417 "memtext.c"
	if ( (*p) == 32 )
		goto tr22;
	goto st0;
tr22:
#line 88 "memtext.rl"
	{
		SET_UINT(exptime, exptime, p);
	}
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 431 "memtext.c"
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr23;
	goto st0;
tr23:
#line 92 "memtext.rl"
	{
		MARK(bytes, p);
	}
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 445 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr24;
		case 32: goto tr25;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st12;
	goto st0;
tr24:
#line 95 "memtext.rl"
	{
		SET_UINT(bytes, bytes, p);
	}
	goto st13;
tr156:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 469 "memtext.c"
	if ( (*p) == 10 )
		goto tr27;
	goto st0;
tr27:
#line 110 "memtext.rl"
	{
		MARK(data_pos, p+1);
		ctx->data_count = ctx->bytes;
		{stack[top++] = 14; goto st130;}
	}
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 485 "memtext.c"
	if ( (*p) == 13 )
		goto st15;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( (*p) == 10 )
		goto tr29;
	goto st0;
tr29:
#line 159 "memtext.rl"
	{
		CALLBACK(cb, memtext_callback_storage);
		if(cb) {
			memtext_request_storage req = {
				MARK_PTR(key_pos[0]), ctx->key_len[0],
				MARK_PTR(data_pos), ctx->data_len,
				ctx->flags,
				ctx->exptime,
				ctx->noreply
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
tr51:
#line 175 "memtext.rl"
	{
		CALLBACK(cb, memtext_callback_cas);
		if(cb) {
			memtext_request_cas req = {
				MARK_PTR(key_pos[0]), ctx->key_len[0],
				MARK_PTR(data_pos), ctx->data_len,
				ctx->flags,
				ctx->exptime,
				ctx->noreply,
				ctx->cas_unique
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
tr78:
#line 205 "memtext.rl"
	{
		CALLBACK(cb, memtext_callback_numeric);
		if(cb) {
			memtext_request_numeric req = {
				MARK_PTR(key_pos[0]), ctx->key_len[0],
				ctx->cas_unique, ctx->noreply
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
tr96:
#line 192 "memtext.rl"
	{
		CALLBACK(cb, memtext_callback_delete);
		if(cb) {
			memtext_request_delete req = {
				MARK_PTR(key_pos[0]), ctx->key_len[0],
				ctx->exptime, ctx->noreply
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
tr118:
#line 140 "memtext.rl"
	{
		unsigned int i;
		++ctx->keys;
		for(i=0; i < ctx->keys; ++i) {
			ctx->key_pos[i] = (size_t)MARK_PTR(key_pos[i]);
		}
		CALLBACK(cb, memtext_callback_retrieval);
		if(cb) {
			memtext_request_retrieval req = {
				(const char**)ctx->key_pos,
				ctx->key_len,
				ctx->keys
			};
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
tr146:
#line 218 "memtext.rl"
	{
		CALLBACK(cb, memtext_callback_other);
		if(cb) {
			memtext_request_other req;
			if((*cb)(ctx->user, ctx->command, &req) < 0) {
				goto convert_error;
			}
		} else { goto convert_error; }
	}
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
#line 600 "memtext.c"
	switch( (*p) ) {
		case 97: goto tr0;
		case 99: goto tr2;
		case 100: goto tr3;
		case 103: goto tr4;
		case 105: goto tr5;
		case 112: goto tr6;
		case 114: goto tr7;
		case 115: goto tr8;
		case 118: goto tr9;
	}
	goto st0;
tr2:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 625 "memtext.c"
	if ( (*p) == 97 )
		goto st17;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( (*p) == 115 )
		goto tr31;
	goto st0;
tr31:
#line 133 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_CAS;     }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 644 "memtext.c"
	if ( (*p) == 32 )
		goto st19;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto st0;
	}
	goto tr33;
tr33:
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 669 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto tr35;
	}
	goto st20;
tr35:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 687 "memtext.c"
	if ( (*p) == 48 )
		goto tr36;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr37;
	goto st0;
tr36:
#line 78 "memtext.rl"
	{
		MARK(flags, p);
	}
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 703 "memtext.c"
	if ( (*p) == 32 )
		goto tr38;
	goto st0;
tr38:
#line 81 "memtext.rl"
	{
		SET_UINT(flags, flags, p);
	}
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 717 "memtext.c"
	if ( (*p) == 48 )
		goto tr39;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr40;
	goto st0;
tr39:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 733 "memtext.c"
	if ( (*p) == 32 )
		goto tr41;
	goto st0;
tr41:
#line 88 "memtext.rl"
	{
		SET_UINT(exptime, exptime, p);
	}
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 747 "memtext.c"
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr42;
	goto st0;
tr42:
#line 92 "memtext.rl"
	{
		MARK(bytes, p);
	}
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 761 "memtext.c"
	if ( (*p) == 32 )
		goto tr43;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st26;
	goto st0;
tr43:
#line 95 "memtext.rl"
	{
		SET_UINT(bytes, bytes, p);
	}
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 777 "memtext.c"
	if ( (*p) == 48 )
		goto tr45;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr46;
	goto st0;
tr45:
#line 103 "memtext.rl"
	{
		MARK(cas_unique, p);
	}
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 793 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr47;
		case 32: goto tr48;
	}
	goto st0;
tr47:
#line 106 "memtext.rl"
	{
		SET_ULL(cas_unique, cas_unique, p);
	}
	goto st29;
tr61:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 815 "memtext.c"
	if ( (*p) == 10 )
		goto tr49;
	goto st0;
tr49:
#line 110 "memtext.rl"
	{
		MARK(data_pos, p+1);
		ctx->data_count = ctx->bytes;
		{stack[top++] = 30; goto st130;}
	}
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 831 "memtext.c"
	if ( (*p) == 13 )
		goto st31;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( (*p) == 10 )
		goto tr51;
	goto st0;
tr48:
#line 106 "memtext.rl"
	{
		SET_ULL(cas_unique, cas_unique, p);
	}
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 852 "memtext.c"
	switch( (*p) ) {
		case 13: goto st29;
		case 32: goto st33;
		case 110: goto st34;
	}
	goto st0;
tr62:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 869 "memtext.c"
	switch( (*p) ) {
		case 13: goto st29;
		case 32: goto st33;
	}
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( (*p) == 111 )
		goto st35;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( (*p) == 114 )
		goto st36;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( (*p) == 101 )
		goto st37;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) == 112 )
		goto st38;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( (*p) == 108 )
		goto st39;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( (*p) == 121 )
		goto st40;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 13: goto tr61;
		case 32: goto tr62;
	}
	goto st0;
tr46:
#line 103 "memtext.rl"
	{
		MARK(cas_unique, p);
	}
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 936 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr47;
		case 32: goto tr48;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st41;
	goto st0;
tr40:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 954 "memtext.c"
	if ( (*p) == 32 )
		goto tr41;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st42;
	goto st0;
tr37:
#line 78 "memtext.rl"
	{
		MARK(flags, p);
	}
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 970 "memtext.c"
	if ( (*p) == 32 )
		goto tr38;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st43;
	goto st0;
tr3:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 988 "memtext.c"
	if ( (*p) == 101 )
		goto st45;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	switch( (*p) ) {
		case 99: goto st46;
		case 108: goto st62;
	}
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( (*p) == 114 )
		goto tr69;
	goto st0;
tr69:
#line 136 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_DECR;    }
	goto st47;
tr124:
#line 135 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_INCR;    }
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
#line 1020 "memtext.c"
	if ( (*p) == 32 )
		goto st48;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto st0;
	}
	goto tr71;
tr71:
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 1045 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto tr73;
	}
	goto st49;
tr73:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 1063 "memtext.c"
	if ( (*p) == 48 )
		goto tr74;
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr75;
	goto st0;
tr74:
#line 103 "memtext.rl"
	{
		MARK(cas_unique, p);
	}
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 1079 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr76;
		case 32: goto tr77;
	}
	goto st0;
tr76:
#line 106 "memtext.rl"
	{
		SET_ULL(cas_unique, cas_unique, p);
	}
	goto st52;
tr86:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1101 "memtext.c"
	if ( (*p) == 10 )
		goto tr78;
	goto st0;
tr77:
#line 106 "memtext.rl"
	{
		SET_ULL(cas_unique, cas_unique, p);
	}
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 1115 "memtext.c"
	if ( (*p) == 110 )
		goto st54;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( (*p) == 111 )
		goto st55;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( (*p) == 114 )
		goto st56;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 101 )
		goto st57;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 112 )
		goto st58;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 108 )
		goto st59;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 121 )
		goto st60;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( (*p) == 13 )
		goto tr86;
	goto st0;
tr75:
#line 103 "memtext.rl"
	{
		MARK(cas_unique, p);
	}
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 1178 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr76;
		case 32: goto tr77;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st61;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( (*p) == 101 )
		goto st63;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( (*p) == 116 )
		goto st64;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( (*p) == 101 )
		goto tr90;
	goto st0;
tr90:
#line 134 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_DELETE;  }
	goto st65;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
#line 1215 "memtext.c"
	if ( (*p) == 32 )
		goto st66;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto st0;
	}
	goto tr92;
tr92:
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1240 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto tr94;
		case 32: goto tr95;
	}
	goto st67;
tr94:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st68;
tr100:
#line 88 "memtext.rl"
	{
		SET_UINT(exptime, exptime, p);
	}
	goto st68;
tr108:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st68;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
#line 1270 "memtext.c"
	if ( (*p) == 10 )
		goto tr96;
	goto st0;
tr95:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 1284 "memtext.c"
	switch( (*p) ) {
		case 48: goto tr97;
		case 110: goto st72;
	}
	if ( 49 <= (*p) && (*p) <= 57 )
		goto tr98;
	goto st0;
tr97:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 1302 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr100;
		case 32: goto tr101;
	}
	goto st0;
tr101:
#line 88 "memtext.rl"
	{
		SET_UINT(exptime, exptime, p);
	}
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 1318 "memtext.c"
	if ( (*p) == 110 )
		goto st72;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 111 )
		goto st73;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 114 )
		goto st74;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 101 )
		goto st75;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 112 )
		goto st76;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( (*p) == 108 )
		goto st77;
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 121 )
		goto st78;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) == 13 )
		goto tr108;
	goto st0;
tr98:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st79;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
#line 1381 "memtext.c"
	switch( (*p) ) {
		case 13: goto tr100;
		case 32: goto tr101;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st79;
	goto st0;
tr4:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st80;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
#line 1401 "memtext.c"
	if ( (*p) == 101 )
		goto st81;
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( (*p) == 116 )
		goto tr111;
	goto st0;
tr111:
#line 126 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_GET;     }
	goto st82;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
#line 1420 "memtext.c"
	switch( (*p) ) {
		case 32: goto st83;
		case 115: goto tr113;
	}
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st0;
		case 32: goto st0;
	}
	goto tr114;
tr114:
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st84;
tr119:
#line 71 "memtext.rl"
	{
		++ctx->keys;
		if(ctx->keys > MEMTEXT_MAX_MULTI_GET) {
			goto convert_error;
		}
	}
#line 65 "memtext.rl"
	{
		MARK(key_pos[ctx->keys], p);
	}
	goto st84;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
#line 1460 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto tr116;
		case 32: goto tr117;
	}
	goto st84;
tr116:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st85;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
#line 1478 "memtext.c"
	if ( (*p) == 10 )
		goto tr118;
	goto st0;
tr117:
#line 68 "memtext.rl"
	{
		SET_MARK_LEN(key_len[ctx->keys], key_pos[ctx->keys], p);
	}
	goto st86;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
#line 1492 "memtext.c"
	switch( (*p) ) {
		case 0: goto st0;
		case 10: goto st0;
		case 13: goto st85;
		case 32: goto st87;
	}
	goto tr119;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	switch( (*p) ) {
		case 13: goto st85;
		case 32: goto st87;
	}
	goto st0;
tr113:
#line 127 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_GETS;    }
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
#line 1517 "memtext.c"
	if ( (*p) == 32 )
		goto st83;
	goto st0;
tr5:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
#line 1533 "memtext.c"
	if ( (*p) == 110 )
		goto st90;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( (*p) == 99 )
		goto st91;
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( (*p) == 114 )
		goto tr124;
	goto st0;
tr6:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st92;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
#line 1563 "memtext.c"
	if ( (*p) == 114 )
		goto st93;
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( (*p) == 101 )
		goto st94;
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( (*p) == 112 )
		goto st95;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( (*p) == 101 )
		goto st96;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( (*p) == 110 )
		goto st97;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( (*p) == 100 )
		goto tr130;
	goto st0;
tr7:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st98;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
#line 1614 "memtext.c"
	if ( (*p) == 101 )
		goto st99;
	goto st0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( (*p) == 112 )
		goto st100;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( (*p) == 108 )
		goto st101;
	goto st0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( (*p) == 97 )
		goto st102;
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( (*p) == 99 )
		goto st103;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 101 )
		goto tr136;
	goto st0;
tr8:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st104;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
#line 1665 "memtext.c"
	if ( (*p) == 101 )
		goto st105;
	goto st0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	if ( (*p) == 116 )
		goto tr138;
	goto st0;
tr9:
#line 59 "memtext.rl"
	{
		ctx->keys = 0;
		ctx->noreply = false;
		ctx->exptime = 0;
	}
	goto st106;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
#line 1688 "memtext.c"
	if ( (*p) == 101 )
		goto st107;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( (*p) == 114 )
		goto st108;
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( (*p) == 115 )
		goto st109;
	goto st0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( (*p) == 105 )
		goto st110;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( (*p) == 111 )
		goto st111;
	goto st0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( (*p) == 110 )
		goto tr144;
	goto st0;
tr144:
#line 137 "memtext.rl"
	{ ctx->command = MEMTEXT_CMD_VERSION; }
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
#line 1735 "memtext.c"
	if ( (*p) == 13 )
		goto st113;
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( (*p) == 10 )
		goto tr146;
	goto st0;
tr25:
#line 95 "memtext.rl"
	{
		SET_UINT(bytes, bytes, p);
	}
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
#line 1756 "memtext.c"
	switch( (*p) ) {
		case 13: goto st13;
		case 32: goto st115;
		case 110: goto st116;
	}
	goto st0;
tr157:
#line 99 "memtext.rl"
	{
		ctx->noreply = true;
	}
	goto st115;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
#line 1773 "memtext.c"
	switch( (*p) ) {
		case 13: goto st13;
		case 32: goto st115;
	}
	goto st0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	if ( (*p) == 111 )
		goto st117;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( (*p) == 114 )
		goto st118;
	goto st0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	if ( (*p) == 101 )
		goto st119;
	goto st0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( (*p) == 112 )
		goto st120;
	goto st0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( (*p) == 108 )
		goto st121;
	goto st0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	if ( (*p) == 121 )
		goto st122;
	goto st0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	switch( (*p) ) {
		case 13: goto tr156;
		case 32: goto tr157;
	}
	goto st0;
tr21:
#line 85 "memtext.rl"
	{
		MARK(exptime, p);
	}
	goto st123;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
#line 1840 "memtext.c"
	if ( (*p) == 32 )
		goto tr22;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st123;
	goto st0;
tr18:
#line 78 "memtext.rl"
	{
		MARK(flags, p);
	}
	goto st124;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
#line 1856 "memtext.c"
	if ( (*p) == 32 )
		goto tr19;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto st124;
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( (*p) == 112 )
		goto st126;
	goto st0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	if ( (*p) == 101 )
		goto st127;
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( (*p) == 110 )
		goto st128;
	goto st0;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	if ( (*p) == 100 )
		goto tr163;
	goto st0;
tr164:
#line 115 "memtext.rl"
	{
		if(--ctx->data_count == 0) {
			//printf("mark %d\n", ctx->data_pos);
			//printf("fpc %p\n", fpc);
			//printf("data %p\n", data);
			SET_MARK_LEN(data_len, data_pos, p+1);
			{cs = stack[--top];goto _again;}
		}
	}
	goto st130;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
#line 1906 "memtext.c"
	goto tr164;
	}
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}
#line 346 "memtext.rl"

ret:
	ctx->cs = cs;
	ctx->top = top;
	*off = p - data;

	if(cs == memtext_error) {
		return -1;
	} else if(cs == memtext_first_final) {
		return 1;
	} else {
		return 0;
	}

convert_error:
	cs = memtext_error;
	goto ret;
}

