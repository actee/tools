/*
	Terminal based 1 one 1 chat
	Host
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "chat.h"

int flow;

void cut(int dummy)
{
	flow=0;
}

int main(int argc,char**argv)
{
	if(argc<2)
	{
		printf("Usage: %s <ip>\n",argv[0]);
		return 1;
	}
	int s,client;
	struct sockaddr_in server;
	char*buffer;
	struct timeval t;
	struct t_args args;
	pthread_t r_t,s_t;
	pthread_mutex_t mtx;
	memset(&server,0,sizeof server);
	memset(&t,0,sizeof t);
	if((server.sin_addr.s_addr=inet_addr(argv[1]))==-1)
	{
		printf("Invalid IP\n");
		return 1;
	}
	server.sin_family=AF_INET;
	server.sin_port = htons(PORT);
	buffer=(char*)malloc(256);
	if(buffer==NULL)
	{
		printf("Error allocationg memory\n");
		return 1;
	}
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==-1)
	{
		printf("Error creating socket\n");
		free(buffer);
		return 1;
	}
	if(bind(s,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("Could not bind listener\n");
		close(s);
		free(buffer);
		return 1;
	}
	listen(s,1);
	client=accept(s,NULL,NULL);
	printf("Accepted\n");
	t.tv_sec=1;
	t.tv_usec=0;
	if(setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof t)<0)
	{
		printf("Could not set timeout\n");
		free(buffer);
		close(client);
		close(s);
		return 1;
	}
	if(setsockopt(client,SOL_SOCKET,SO_SNDTIMEO,&t,sizeof t)<0)
	{
		printf("Could not set timeout\n");
		free(buffer);
		close(client);
		close(s);
		return 1;
	}
	args.sock=&client;
	args.buffer=&buffer;
	args.mutex=&mtx;
	args.sig=&flow;
	pthread_mutex_init(&mtx,NULL);
	flow=1;
	signal(SIGINT,cut);
	printf("Pointers:\n\tinput buffer %p\n\tmutex %p\n\tflow %p\n\tsocket %p\n",*(args.buffer),args.mutex,args.sig,args.sock);
	if(pthread_create(&r_t,NULL,recv_t,(void*)&args)||pthread_create(&s_t,NULL,send_t,(void*)&args))
	{
		printf("Error creating threads\n");
		free(buffer);
		close(s);
		close(client);
		return 1;
	}
	pthread_mutex_destroy(&mtx);
	pthread_join(r_t,NULL);
	pthread_join(s_t,NULL);
	free(buffer);
	close(client);
	close(s);
	return 0;
}
