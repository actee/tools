/*
 * Command line parser for
 * HTTP Digest Access Authentication Cracker
 *
 * Header file
 *
 *
 * Copyright Tiago Teixeira, 2017
 *
*/

#include <stdio.h>

#ifndef _PARSER_DIGEST_H
#define _PARSER_DIGEST_H

struct args
{
	char*hostname;
	short port;	// default 80
	FILE*userf;
	FILE*passf;
	int verbose;	// default 0
};

int parse_args(int argc,char**argv,struct args *args);
void help(const char*prog);

#endif
