/*
 connect to a server and exchange messages
 
 may be useful to identify services and/or test payloads
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<signal.h>
#include	<time.h>
#define	BSIZ	2048
#define STDIN	0
int parse_input(const char*in,char*out,int size);
void handler(int);
int sig;
int main(int argc,char**argv)
{
	if(argc<3)
	{
		printf("Usage: %s <IP> <port> [hexdump]\n",argv[0]);
		return 0;
	}
	int sock,i_siz,f_siz,buf_len,i,hex=0;
	short port;
	char c;
	struct sockaddr_in server;
	char *input,*output;
	struct timeval timeout;
	if(argc==4)
		hex=1;
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock==-1)
	{
		printf("Error opening socket\n");
		return 1;
	}
	if(inet_addr(argv[1])==-1)
	{
		printf("IP is invalid\n");
		return 1;
	}
	if((port=(short)atoi(argv[2]))==0)
	{
		printf("Invalid port\n");
		return 1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(port);
	memset(&timeout,0,sizeof timeout);
	timeout.tv_sec=2;
	timeout.tv_usec=0;
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof timeout)<0)
	{
		printf("Error setting timeout\n");
		close(sock);
		return 1;
	}
	input = (char*)malloc(BSIZ);
	output= (char*)malloc(BSIZ);
	if(input==NULL||output==NULL)
	{
		printf("Error allocating memory\n");
		if(input!=NULL)
			free(input);
		if(output!=NULL)
			free(output);
		close(sock);
		return 1;
	}
	printf("Connecting...\n");
	if(connect(sock,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("Error connecting\n");
		close(sock);
		free(input);
		free(output);
		return 1;
	}
	sig=1;
	signal(SIGINT,handler);
	printf("Press CTRL+C to exit\n");
	while(sig)
	{
		printf("--------------------Attempting to receive...--------------------\n");
		memset(input,0,BSIZ);
		buf_len=recv(sock,input,BSIZ,0);
		if(buf_len==-1)
			printf("No inbound data\n");
		else
		{
			if(hex)
				for(i=0;i<buf_len;i++)
					printf("x%02hhx ",*(input+i));
			else
				for(i=0;i<buf_len;i++)
					putchar(*(input+i));
		}
		printf("\n----------------------------------------------------------------\n");
		memset(input,0,BSIZ);
		printf("Input payload >>\n");
		i_siz=read(STDIN,input,BSIZ);
		f_siz=parse_input(input,output,i_siz);
		if(send(sock,output,f_siz,0)<0)
		{
			printf("Error sending\n");
			printf("Quit? [Y/n] ");
			scanf(" %c",&c);
			if(c!='n'&&c!='N')
				sig=0;
		}
	}
	free(input);
	free(output);
	close(sock);
	return 0;
}
int parse_input(const char*input,char*output,int i_siz)
{
	int siz,i,s;
	memset(output,0,BSIZ);
	for(i=0,siz=0,s=0;i<i_siz;i++)
	{
		if(s)
		{
			switch(*(input+i))
			{
				case 'n':
					*output++='\n';
					siz++;
					break;
				case 'r':
					*output++='\r';
					siz++;
					break;
				case 't':
					*output++='\t';
					siz++;
					break;
				case '0':
					*output++='\0';
					siz++;
					break;
				case '\\':
					*output++='\\';
					siz++;
					break;
				case '\'':
					*output++='\'';
					siz++;
					break;
				default:
					*output++='\\';
					siz++;
					break;
			}
			s=0;
		}
		else if(*(input+i)=='\\')
			s=1;
		else
		{
			*output++=*(input+i);
			siz++;
		}
	}
	return siz;
}
void handler(int dummy)
{
	sig=0;
}
