/*
 Core file for terminal chat
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include "chat.h"
void*recv_t(void*in)
{
	struct t_args*args=(struct t_args*)in;
	char*buffer=(char*)malloc(256);
	int i;
	while(*(args->sig))
	{
		memset(buffer,0,256);
		pthread_mutex_lock(args->mutex);
		if(recv(*(args->sock),buffer,256,0)>0)
		{
			printf("\r");fflush(stdout);
			for(i=0;i<strlen(*(args->buffer));i++)
			{
				printf(" ");
				fflush(stdout);
			}
			printf("\r%s\n%s",buffer,*(args->buffer));
			fflush(stdout);
		}
		pthread_mutex_unlock(args->mutex);
	}
	free(buffer);
	return in;
}
void*send_t(void*in)
{
	struct t_args*args=(struct t_args*)in;
	char*input=*(args->buffer);
	char c;
	int len;
	memset(input,0,256);
	while(*(args->sig))
	{
		len=0;
		while((c=getchar())!='\n')
			*(input+len++)=c;
		pthread_mutex_lock(args->mutex);
		if(send(*(args->sock),input,len,0)<0)
		{
			printf("Error sending data\n");
			fflush(stdout);
		}
		memset(input,0,256);
		pthread_mutex_unlock(args->mutex);
	}
	return in;
}
