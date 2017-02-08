/*
	Terminal based chat header declarations
*/
#ifndef		CHAT_HEADER_MINE
#define		CHAT_HEADER_MINE
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<pthread.h>
#define		PORT	5555
#define		OK	1
#define		NOK	0
struct t_args
{
	int*sock;
	char**buffer;
	pthread_mutex_t*mutex;
	int*sig;
};
void*recv_t(void*);
void*send_t(void*);
void cut(int);
#endif
