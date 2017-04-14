/*
 * HTTP Basic Access Authentication Cracker
 * Header file for arguments parser
 *
 *
 * Copyright Tiago Teixeira, 2017
 *
*/
#ifndef _HTTP_BAA_PARSER_H_
#define _HTTP_BAA_PARSER_H_
#include <stdio.h>
struct baa_args
{
	char*	hostname;	// points to the correspondent argv[?]
	short	port;		// default 80
	FILE*	userf;		// the already opened files
	FILE*	passf;		// for users and passwords
	int	verb;		// verbose switch
};
int	parse_args(int argc,char**argv,struct baa_args* parsed_args);
void	usage(const char*prog_name);
#endif