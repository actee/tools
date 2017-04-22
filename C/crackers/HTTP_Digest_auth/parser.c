/*
 * Command line parser for
 * HTTP Digest Access Authentication Cracker
 *
 * Source file
 *
 *
 * Copyright Tiago Teixeira, 2017
 *
*/

#include "parser.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void help(const char*str)
{
	printf("usage: %s -H <hostname> -u <userfile> -p <passfile> [-P <port=80>] [-v]\n",str);
	printf("\n--help\t\t-h\tshows this message\n");
	printf("\n--hostname\t-H\tHostname or IP of target [REQUIRED]\n");
	printf("--user\t\t-u\tfile with users [REQUIRED]\n");
	printf("--pass\t\t-p\tfile with password [REQUIRED]\n");
	printf("--verbose\t-v\tuses verbose mode, default none\n");
	printf("--port\t\t-P\tsets port to use, default is 80\n");
}

int parse_args(int argc,char**argv,struct args *args)
{
	if(argc < 7)
	{
		help(argv[0]);
		return 1;
	}
	int i;
	// help first
	for(i=1;i<argc;i++)
	{
		if(!strcmp("--help",argv[i])||!strcmp("-h",argv[i]))
		{
			help(argv[0]);
			return 1;
		}
	}
	// hostname
	for(i=1;i<(argc-1);i++)
		if(!strcmp("-H",argv[i])||!strcmp("--host",argv[i]))
		{
			args->hostname = argv[i+1];
			break;
		}
	if(i==(argc-1))
		return 1;	// reached max level, error
	// passfile
	for(i=1;i<(argc-1);i++)
		if(!strcmp("--pass",argv[i])||!strcmp("-p",argv[i]))
		{
			if(access(argv[i+1],R_OK))
			{
				fprintf(stderr,"Password file is not readable\n");
				return 1;
			}
			args->passf = fopen(argv[i+1],"rb");
			if(args->passf==NULL)
			{
				fprintf(stderr,"Could not open password file\n");
				return 1;
			}
			break;
		}
	if(i==(argc-1))
		return 1; // error here
	// userfile
	for(i=1;i<(argc-1);i++)
		if(!strcmp("--user",argv[i])||!strcmp("-u",argv[i]))
		{
			if(access(argv[i+1],R_OK))
			{
				fprintf(stderr,"User file is not readable\n");
				fclose(args->passf);
				return 1;
			}
			args->userf=fopen(argv[i+1],"rb");
			if(args->userf==NULL)
			{
				fprintf(stderr,"Could not open user file\n");
				fclose(args->passf);
				return 1;
			}
			break;
		}
	if(i==(argc-1))
		return 1;
	
	// port
	args->port = 80;
	for(i=1;i<(argc-1);i++)
		if(!strcmp("--port",argv[i])||!strcmp("-P",argv[i]))
		{
			args->port = (short)atoi(argv[i+1]);
			break;
		}
	
	// verbose
	args->verbose = 0;
	for(i=1;i<argc;i++)
		if(!strcmp("--verbose",argv[i])||!strcmp("-v",argv[i]))
		{
			args->verbose=1;
			break;
		}
	// all good
	return 0;
}
