/*
 * dns resolver
 *
 * Copyright Tiago Teixeira, 2017
 *
*/

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

int main(int argc,char**argv)
{
	if(argc!=2)
	{
		fprintf(stderr,"usage: %s <hostname>\n",argv[0]);
		return 1;
	}
	struct addrinfo hints;
	struct addrinfo*r;
	struct sockaddr_in *res;
	int sig;
	unsigned long addr;
	memset(&hints,0,sizeof hints);
	hints.ai_family		=AF_INET;
	hints.ai_socktype	=SOCK_STREAM;
	hints.ai_addr		=NULL;
	hints.ai_canonname	=NULL;
	hints.ai_next		=NULL;
	hints.ai_flag		=0;
	hints.ai_protocol	=0;
	
	sig = getaddrinfo(argv[1],NULL,&hints,&r);
	if(sig)
	{
		printf("Could not resolve\n");
		return 1;
	}
	res = (struct sockaddr_in*)r->ai_addr;
	
	addr = res->sin_addr.s_addr;
	printf("%lu.%lu.%lu.%lu\n",addr%256,(addr>>8)%256,(addr>>16)%256,(addr>>24)%256);
	freeaddrinfo(r);
	return 0;
}
