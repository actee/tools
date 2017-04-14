/*
 * HTTP Basic Access Authentication Cracker
 * Source file for arguments parser
 *
 *
 * Copyright Tiago Teixeira, 2017
 *
*/
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int parse_args(int argc,char**argv,struct baa_args*parsed)
{
	if(argc<7)	// Number of required argc
	{
		usage(argv[0]);
		return 1;
	}
	int i;
	// look for help
	for(i=1;i<argc;i++)
	{
		if(!strcmp(argv[i],"-h")||!strcmp(argv[i],"-h"))
		{
			usage(argv[0]);
			return 1;
		}
	}
	// look for hostname
	for(i=1;i<(argc-1);i++)
	{
		if(!strcmp(argv[i],"--host")||!strcmp(argv[i],"-H"))
		{
			parsed->hostname=argv[i+1];
			i=0;
			break;
		}
	}
	if(i)
	{
		fprintf(stderr,"hostname if required\n");
		return 1;
	}
	// look for user file
	for(i=1;i<(argc-1);i++)
	{
		if(!strcmp(argv[i],"--user")||!strcmp(argv[i],"-u"))
		{
			if(access(argv[i+1],R_OK))
			{
				fprintf(stderr,"invalid user file\n");
				return 1;
			}
			parsed->userf=fopen(argv[i+1],"rb");
			if(parsed->userf==NULL)
			{
				fprintf(stderr,"invalid user file\n");
				return 1;
			}
			i=0;
			break;
		}
	}
	if(i)
	{
		fprintf(stderr,"usernames file is required\n");
		return 1;
	}
	// look for pass file
	for(i=1;i<(argc-1);i++)
	{
		if(!strcmp(argv[i],"--pass")||!strcmp(argv[i],"-p"))
		{
			if(access(argv[i+1],R_OK))
			{
				fprintf(stderr,"invalid pass file\n");
				return 1;
			}
			parsed->passf=fopen(argv[i+1],"rb");
			if(parsed->userf==NULL)
			{
				fclose(parsed->userf);
				fprintf(stderr,"invalid pass file\n");
				return 1;
			}
			i=0;
			break;
		}
	}
	if(i)
	{
		fprintf(stderr,"passwords file is required\n");
		return 1;
	}
	// look for port
	parsed->port=(short)80;	// default
	for(i=1;i<(argc-1);i++)
	{
		if(!strcmp(argv[i],"--port")||!strcmp(argv[i],"-P"))
		{
			parsed->port = (short)atoi(argv[i+1]);
			printf("using custom port %hd\n",parsed->port);
			break;
		}
	}
	// look for verbose
	parsed->verb=0;		// default
	for(i=1;i<argc;i++)
	{
		if(!strcmp(argv[i],"--verbose")||!strcmp(argv[i],"-v"))
		{
			parsed->verb=1;
			break;
		}
	}
	// all good
	return 0;
}

void usage(const char*p)
{
	printf("usage: %s -H <hostname> -u <userfile> -p <passfile> [-P <port=80>] [-v]\n\n",p);
	printf("help\n");
	printf("\t--host\t\t-H\t- target hostname\t[REQUIRED]\n");
	printf("\t--user\t\t-u\t- usernames dictionary file\t[REQUIRED]\n");
	printf("\t--pass\t\t-p\t- passwords dictionary file\t[REQUIRED]\n");
	printf("\t--port\t\t-P\t- port to communicate with, default 80\n");
	printf("\t--verbose\t-v\t- verbose mode, default no\n");
	printf("\t--help\t\t-h\t- displays this message\n");
}