/*
 Main file for hgame
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "header.h"

int main(int argc, char**argv)
{
	/* Booting messages and stuff */
	system("clear");
	printf("Booting hgameOS...\n");
	int cmd_op,own_ip,cur_ip,tgt_ip;
	int buf_len;
	int sock;
	char user[16];
	struct timeval tv;
	printf("Initializing connections...\n");
	struct sockaddr_in server;
	printf("Allocating memory...\n");
	struct cmd command;
	unsigned char *buffer = (unsigned char*)malloc(1024);
	char *output =(char*)malloc(1024);
	if(buffer==NULL || output==NULL)
	{
		printf("Error allocating memory\nExiting...\n");
		return 1;
	}
	memset(buffer,0,1024);
	memset(output,0,1024);
	printf("Memory allocated\n");
	sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);	// again with TCP
	if(sock==-1)
	{
		printf("Could not initialize socket\n");
		free(buffer);
		free(output);
		return 1;
	}
	memset(&server,0,sizeof server);
	printf("Initializing Network...\n");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(SERVER);
	server.sin_port = htons(S_PORT);
	tv.tv_sec=3;
	tv.tv_usec=0;
	if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof tv)<0)
	{
		printf("Error initializing connections\nExiting...\n");
		free(buffer);
		free(output);
		return 1;
	}
	if(connect(sock,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("Network Error\nShutting down...\n");
		free(buffer);
		free(output);
		close(sock);
		return 1;
	}
	own_ip = auth(&sock,buffer);
	if(own_ip==-1)
	{
		free(buffer);
		free(output);
		shutdown(sock,SHUT_RDWR);
		return 1;
	}
	strcpy(user,(char*)buffer);
	memset(buffer,0,1024);
	cur_ip=own_ip;
	tgt_ip=own_ip;
	/* Welcome message */
	printf("Welcome to the hgameOS\n");
	printf("Type help if you don't know what to do\n");
	do
	{
		memset(&command,0,sizeof command);
		prompt(user,cur_ip);
		memset(output,0,1024);
		cmd_op=parse_cmd(&command,output);
		if(cmd_op==ERROR)
		{
			if((cmd_op=exec(&sock,own_ip,buffer,output))==CONERR)
			{
				printf("Networking Error\n");
				cmd_op=POWOFF;
			}
		}
		switch(cmd_op)
		{
			case ERROR:
				printf("Unknown Command\n");
				break;
			case CONNECT:
				tgt_ip=command.cmd_opt1==LCLHOST?own_ip:command.cmd_opt1;
				printf("Linked to %hhu.%hhu.%hhu.%hhu\n",(char)(tgt_ip%256),(char)((tgt_ip>>8)%256),(char)((tgt_ip>>16)%256),(char)((tgt_ip>>24)%256));
				break;
			case LOGOUT:
				cur_ip=own_ip;
				tgt_ip=cur_ip;
				break;
			case POWOFF:
				printf("Shutting down...\n");
				break;
			case HELP:
				printf("Available Commands:\n");
				printf("active\t\t\tsees active services\n");
				printf("clear\t\t\tclears screen\n");
				printf("connect <ip>\t\tconnects to ip\n");
				printf("crack <port> <'key'>\tcrack on the go, instead of using an exploit (RTFM)\n");
				printf("create\t\t\texecutes the software creation tool\n");
				printf("download <filename>\tor \'dl\' downloads from connected host to local machine\n");
				printf("fuzz <port>\t\tFuzzes the target service\n");
				printf("kill <filename>\t\tstops process <filename>\n");
				printf("logout\t\t\tlogs out of the host\n");
				printf("ls\t\t\tlist files and directories in host\n");
				printf("help\t\t\tdisplays this message\n");
				printf("nmap\t\t\tsame as 'scan'\n");
				printf("poweroff\t\tshuts down the console\n");
				printf("rm <filename>\t\tremoves file at connected host\n");
				printf("scan\t\t\tscans for open ports/services\n");
				printf("start <filename>\tstarts process <filename> on host\n");
				printf("upload <filename>\tor \'ul\'uploads from local machine to connected host\n");
				break;
			case ACTIVE:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=ACTIVE;
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&cur_ip,4);
				buf_len=10;
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				memset(buffer,0,10);
				if(recv(sock,buffer,1024,0)<0)
				{
					printf("Network error\n");
					break;
				}
				if(*buffer==S_CMD&&*(buffer+1))
					printf("%s",(char*)(buffer+2));
				else
					printf("Error listing\n");
				break;
			case CLEAR:
				system("clear");
				break;
			case LS:
				memset(buffer,0,1024);
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&cur_ip,4);
				*buffer=S_CMD;
				*(buffer+1)=LS;
				buf_len=10;
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Can't see directories\n");
					break;
				}
				memset(buffer,0,1024);
				if(recv(sock,buffer,1024,0)<0)
				{
					printf("Can't see directories\n");
					break;
				}
				if(!(*(buffer+1)))
					printf("Can't see directories\n");
				else
					printf("%s",buffer+2);
				break;
			case CREAT:
				memset(buffer,0,1024);
				*buffer = S_CSW;
				memcpy(buffer+1,&own_ip,4);
				buf_len = create_lobby(buffer);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Can't save software\n");
					break;
				}
				memset(buffer,0,1024);
				if(recv(sock,buffer,2,0)<0)
				{
					printf("Can't save software\n");
					break;
				}
				if(*buffer==S_CSW&&*(buffer+1))
					printf("Software saved\n");
				else
					printf("Error saving\n");
				break;
			case EXEC:
				if(tgt_ip==own_ip)
				{
					printf("Need to connect to a host first\n");
					printf("connect <host>\n");
					break;
				}
				memset(buffer,0,1024);
				*buffer=S_CRK;
				memcpy(buffer+1,&own_ip,4);
				memcpy(buffer+5,&tgt_ip,4);
				memcpy((char*)(buffer+9),output,16);
				buf_len=25;
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				memset(buffer,0,64);
				buf_len=6;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				if(*buffer==S_CRK&&*(buffer+1)==1)
					memcpy(&cur_ip,buffer+2,4);
				else
					printf("Could not crack\n");
				break;
			case NMAP:
				memset(buffer,0,1024);
				*buffer=S_RCN;
				memcpy(buffer+1,&tgt_ip,4);
				buf_len = 5;
				printf("Scanning %hhu.%hhu.%hhu.%hhu...\n",(unsigned char)(tgt_ip%256),(unsigned char)((tgt_ip>>8)%256),(unsigned char)((tgt_ip>>16)%256),(unsigned char)((tgt_ip>>24)%256));
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error scanning\n");
					break;
				}
				memset(buffer,0,1024);
				buf_len = 1024;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error scanning\n");
					break;
				}
				parse_nmap(buffer);
				break;
			case START:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=START;
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&cur_ip,4);
				strcpy((char*)(buffer+10),output);
				buf_len=10+strlen(output);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error sending request\n");
					break;
				}
				memset(buffer,0,1024);
				buf_len = 2;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error getting response\n");
					break;
				}
				if(*buffer==S_CMD&&*(buffer+1))
					printf("Process started with success\n");
				else
					printf("Process start failure\n");
				break;
			case KILL:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=KILL;
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&own_ip,4);
				strcpy((char*)(buffer+10),output);
				buf_len=10+strlen(output);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error sending request\n");
					break;
				}
				memset(buffer,0,1024);
				buf_len=2;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error getting response\n");
					break;
				}
				printf("DEBUG %hhu %hhu\n",*buffer,*(buffer+1));
				if(*buffer==S_CMD&&*(buffer+1)==1)
					printf("Process stopped with success\n");
				else
					printf("Process stopping failure\n");
				break;
			case UPLD:
				if(own_ip==cur_ip)
				{
					printf("You can't upload to yourself\n");
					break;
				}
				memset(buffer,0,1024);
				*buffer=S_DUL;
				memcpy(buffer+1,&own_ip,4);
				memcpy(buffer+5,&cur_ip,4);
				strcpy((char*)(buffer+9),output);
				buf_len=9+strlen(output);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error uploading\n");
					break;
				}
				buf_len=2;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error uploading\n");
					break;
				}
				if(*buffer==S_DUL&&*(buffer+1)==1)
					printf("Uploaded\n");
				else
					printf("Error uploading\n");
				break;
			case DNLD:
				if(own_ip==cur_ip)
				{
					printf("You can't download from yourself\n");
					break;
				}
				memset(buffer,0,1024);
				*buffer=S_DUL;
				memcpy(buffer+1,&cur_ip,4);
				memcpy(buffer+5,&own_ip,4);
				strcpy((char*)(buffer+9),output);
				buf_len=9+strlen(output);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error downloading\n");
					break;
				}
				buf_len=2;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error downloading\n");
					break;
				}
				if(*buffer==S_DUL&&*(buffer+1))
					printf("Downloaded\n");
				else
					printf("Error downloading\n");
				break;
			case RM:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=RM;
				memcpy(buffer+4,&own_ip,4);
				memcpy(buffer+6,&cur_ip,4);
				strcpy((char*)(buffer+10),output);
				buf_len=10+strlen(output);
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Error executing command\n");
					break;
				}
				buf_len=2;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Error executing command\n");
					break;
				}
				if(*buffer==S_CMD&&*(buffer+1))
					printf("File removed\n");
				else
					printf("File could not be removed\n");
				break;
			case FUZZ:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=FUZZ;
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&tgt_ip,4);
				*(buffer+10)=command.cmd_opt1;
				buf_len=11;
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				memset(buffer,0,1024);
				buf_len=10;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				if(*buffer==S_CMD&&*(buffer+1))
				{
					printf("Fuzz results:\n");
					printf("%8s\n",(char*)(buffer+2));
				}
				else
					printf("Fuzzing error\n");
				break;
			case CRACK:
				memset(buffer,0,1024);
				*buffer=S_CMD;
				*(buffer+1)=CRACK;
				memcpy(buffer+2,&own_ip,4);
				memcpy(buffer+6,&tgt_ip,4);
				*(buffer+10)=command.cmd_opt1==21?FTP:SSH;
				*(buffer+11)=char_to_op(output[0]);
				*(buffer+12)=char_to_op(output[1]);
				*(buffer+13)=char_to_op(output[2]);
				buf_len=14;
				if(send(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				memset(buffer,0,1024);
				buf_len=6;
				if(recv(sock,buffer,buf_len,0)<0)
				{
					printf("Network error\n");
					break;
				}
				if(*buffer==S_CMD&&*(buffer+1))
				{
					memcpy(&cur_ip,buffer+2,4);
					printf("Cracked\n");
				}
				else
					printf("Could not crack\n");
				break;
			default:
				printf("Unknown command\n");
				break;
		}
	}
	while(cmd_op!=POWOFF);
	close(sock);
	free(buffer);
	free(output);
	return 0;
}

void prompt(const char*uname,int ip)
{
	char sip[16];
	memset(sip,0,16);
	if(ip!=NULL_IP)
		sprintf(sip,"%hhu.%hhu.%hhu.%hhu",(char)(ip%256),(char)((ip>>8)%256),(char)((ip>>16)%256),(char)((ip>>24)%256));
	printf("%s@%s $ ",uname,sip);
}

int auth(int*sock,unsigned char*buffer)
{
	int buf_len,ip,auth=0;
	char usr[16];
	char *op = (char*)buffer;
	char*user = (char*)(buffer+1);
	char*pass = (char*)(buffer+17);
	while(!auth)
	{
		buf_len=33;
		memset(buffer,0,1024);
		printf("Local login: ");
		scanf("%16s",user);
		clean_stdin();
		memcpy(usr,user,16);
		printf("Local password: ");
		scanf("%16s",pass);
		clean_stdin();
		*op = S_AUTH;
		if(send(*sock,buffer,buf_len,0)<0)
		{
			printf("Kernel error on authentication\n");
			printf("Shutting down\n");
			shutdown(*sock,SHUT_RDWR);
			return -1;
		}
		buf_len = 6;	// expected
		if(recv(*sock,buffer,buf_len,0)<0)
		{
			printf("Kernel error on authentication\n");
			printf("Shutting down\n");
			shutdown(*sock,SHUT_RDWR);
			return -1;
		}
		if(*buffer==S_AUTH && *(buffer+1)==1)
		{
			printf("Authenticated\n");
			memcpy(&ip,buffer+2,4);
			auth++;
		}
		else
			printf("Authentication failed\n");
	}
	memcpy((char*)buffer,usr,16);
	return ip;
}
