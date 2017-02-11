/*
 Execute cracks and stuff
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

int exec(int*sock,int own_ip,unsigned char*buffer,const char*fname)
{
	int buf_len;
	buf_len=21;
	memset(buffer,0,1024);
	*buffer=S_CFE;
	memcpy(buffer+1,&own_ip,4);
	memcpy((char*)(buffer+5),fname,16);
	if(send(*sock,buffer,buf_len,0)<0)
	{
		printf("Could not connect to server\n");
		return CONERR;
	}
	memset(buffer,0,buf_len);
	buf_len = 16;
	if(recv(*sock,buffer,buf_len,0)<0)
	{
		printf("Could not connect to server\n");
		return CONERR;
	}
	if(*buffer == S_CFE && *(buffer+1)==1)
		return EXEC;
	else
		return ERROR;
}
unsigned char char_to_op(char c)
{
	unsigned char op;
	if(c=='d')op=DOS;
	else if(c=='i')op=INJ;
	else if(c=='b')op=BOF;
	else if(c=='s')op=SPOOF;
	else if(c=='m')op=MITM;
	else if(c=='e')op=EOP;
	else op=DOS;
	return op;
}
