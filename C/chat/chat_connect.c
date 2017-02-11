/*
	Terminal based 1 one 1 chat
	Connect
	
	Copyright 2017 Tiago Teixeira
	 Distributed under the terms of the GNU General Public License
	 See <http://www.gnu.org/licenses/> for more info
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "chat.h"

int flow;

void cut(int dummy)
{
	flow=0;
}

int main(int argc, char**argv)
{
	if(argc<2)
	{
		printf("Usage: %s <ip>\n",argv[0]);
		return 1;
	}
	int s;
	struct sockaddr_in server;
	char*buffer;
	struct timeval t;
	pthread_t rcv_t,snd_t;
	struct t_args args;
	pthread_mutex_t mutex;
	memset(&server,0,sizeof server);
	memset(&t,0,sizeof t);
	if((server.sin_addr.s_addr=inet_addr(argv[1]))==-1)
	{
		printf("Invalid IP\n");
		return 1;
	}
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==-1)
	{
		printf("Error creating socket\n");
		return 1;
	}
	memset(&t,0,sizeof t);
	t.tv_sec = 1;
	t.tv_usec= 0;
	if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof t)<0)
	{
		printf("Error setting socket timeout\n");
		close(s);
		return 1;
	}
	if(setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,&t,sizeof t)<0)
	{
		printf("Error setting timeout\n");
		close(s);
		return 1;
	}
	buffer=(char*)malloc(256);
	if(buffer==NULL)
	{
		printf("Error allocating memory\n");
		close(s);
		return 1;
	}
	if(connect(s,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("Error connecting\n");
		free(buffer);
		close(s);
		return 1;
	}
	printf("Connected\n");
	args.sock=&s;
	args.buffer=&buffer;
	args.mutex=&mutex;
	args.sig=&flow;
	pthread_mutex_init(&mutex,NULL);
	flow=1;
	signal(SIGINT,cut);
	printf("Pointers:\n\tinput buffer %p\n\tmutex %p\n\tflow %p\n\tsocket %p\n",*(args.buffer),args.mutex,args.sig,args.sock);
	if(pthread_create(&rcv_t,NULL,recv_t,(void*)&args)||pthread_create(&snd_t,NULL,send_t,(void*)&args))
	{
		printf("Threading creation error\n");
		free(buffer);
		close(s);
		return 1;
	}
	pthread_mutex_destroy(&mutex);
	pthread_join(rcv_t,NULL);
	pthread_join(snd_t,NULL);
	free(buffer);
	close(s);
	return 0;
}
