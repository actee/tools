/*
 * HTTP Digest Access Authentication Cracker
 *
 * Header file
 *
 *
 * Copyright Tiago Teixeira, 2017
*/

#ifndef _HTTP_DIGEST_CRK_H_
#define _HTTP_DIGEST_CRK_H_

#define DNS	"dns"	// for hostname resolving, change for own utility or use this

#define QOP_AUTH	0x01
#define QOP_AUTH_INT	0x02
#define OPAQUE		0x04	// not yet implemented
#define P_ERROR		0x80	// last bit

struct digest_info
{
	char info;
	char *realm;
	char *nonce;
};

int dns(const char*name);		// own dns utility
int md5(const char*in,char*out);	// Linux's md5sum utility

struct digest_info parse_http(char*http_header);	// parse http header
void free_info(struct digest_info *info);		// frees recurses used by opaque, realm ...

int readline(FILE*file, char* output);	// file handling

int init_socket(int old);

#endif
