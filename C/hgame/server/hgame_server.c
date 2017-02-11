/*
 hgameOS server
 building/debugging
 purposes
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "decs.h"

int main(void)
{
	int *sock,serv;
	int error;
	int buf_siz,rcv_size,h;
	struct sockaddr_in server,client;
	socklen_t client_len;
	unsigned char buffer[BSIZE];
	int sock_list[8],i,s_am=0,rcv;
	struct timeval tv;
	memset(&tv,0,sizeof tv);
	serv = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);	// again with TCP
	if(serv==-1)
	{
		printf("Error initializing socket\n");
		return 1;
	}
	printf("Socket created\n");
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(SERVER);
	server.sin_port = htons(8000);
	tv.tv_sec=0;
	tv.tv_usec=750000;
	if(bind(serv,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("Could not bind server\n");
		close(serv);
		return 1;
	}
	printf("Socket binded\n");
	rcv=0;
	while(1)
	{
		printf("Listening...\n");
		memset(&client,0,sizeof client);
		memset(buffer,0,BSIZE);
		rcv=0;
		while(!rcv)
		{
			// accepting / recving loop
			listen(serv,1);
			if((sock_list[s_am++]=accept4(serv,NULL,NULL,0))==-1)
				sock_list[--s_am]=0;
			else
			{
				if(setsockopt(sock_list[s_am-1],SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof tv)<0)
				{
					error=errno;
					check_error(error);
					printf("Error setting timout on new socket\n");
					close(sock_list[--s_am]);
				}
				printf("Accepted connection\n");
			}
			for(i=0;i<s_am;i++)
				if((h=recv(sock_list[i],buffer,BSIZE,0))>0)
				{
					sock=&sock_list[i];
					i=s_am;
					rcv=1;
				}
		}
		printf("Received %d bytes\n",h);
		printf("Op code %hhu\n",*buffer);
		switch(*buffer)
		{
			case S_AUTH:
				buf_siz=auth(buffer);
				if(buf_siz==-1)
				{
					memset(buffer,0,BSIZE);
					*buffer=S_AUTH;
					*(buffer+1)=0;
					buf_siz=6;
				}
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_CMD:
				buf_siz=cmd_exec(buffer);
				if(buf_siz==-1)
				{
					memset(buffer,0,BSIZE);
					*buffer=S_CMD;
					*(buffer+1)=0;
					buf_siz=2;
				}
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_RCN:
				buf_siz=nmap(buffer);
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_CFE:
				buf_siz=check_file_exists(buffer);
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_CSW:
				buf_siz=2;
				*(buffer+1)=(char)create_software(buffer);
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_CRK:
				buf_siz=crack(buffer);
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			case S_DUL:
				buf_siz=down_up_load(buffer);
				if(send(*sock,buffer,buf_siz,0)<0)
					printf("Error sending data\n");
				break;
			default:
				printf("Ignoring...\n");
				break;
		}
	}
	return 0;
}
int down_up_load(unsigned char*buffer)
{
	int len;
	char cmd[64];
	int src,dst;
	memcpy(&src,buffer+1,4);
	memcpy(&dst,buffer+5,4);
	sprintf(cmd,"%08x/%s",src,(char*)(buffer+9));
	if(access(cmd,F_OK)==-1)
	{
		len=2;
		memset(buffer,0,BSIZE);
		*buffer=S_DUL;
		*(buffer+1)=0;
		return len;
	}
	sprintf(cmd,"cp %08x/%s %08x/",src,(char*)(buffer+9),dst);
	system(cmd);
	len=2;
	memset(buffer,0,BSIZE);
	*buffer=S_DUL;
	*(buffer+1)=1;
	return len;
}
int crack(unsigned char*buffer)
{
	int crk_ip,srv_ip,len;
	char cmd[64];
	char crk_opts[4];
	char srv_opts[4];
	char dummy;
	char *fname = (char*)(buffer+9);
	int cool=0;
	FILE*f;
	DIR*dir;
	struct dirent *entry;
	memcpy(&crk_ip,buffer+1,4);
	memcpy(&srv_ip,buffer+5,4);
	sprintf(cmd,"%08x/%s",crk_ip,fname);
	f=fopen(cmd,"rb");
	if(f==NULL)
	{
		printf("Error opening file\n");
		*(buffer+1)=0;
		len=2;
		return len;
	}
	fread(&dummy,1,1,f);
	if(dummy!=CRACKER)
	{
		printf("File is not a cracker\n");
		*(buffer+1)=0;
		len=2;
		fclose(f);
		return len;
	}
	fread(crk_opts,1,4,f);
	fclose(f);
	// check if service is also running on attacker
	sprintf(cmd,"%08x/active",crk_ip);
	dir=opendir(cmd);
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",crk_ip,entry->d_name);
		f=fopen(cmd,"rb");
		fseek(f,1,SEEK_SET);
		fread(&dummy,1,1,f);
		if(dummy==*crk_opts)
			cool=1;
		fclose(f);
		if(cool)break;
	}
	closedir(dir);
	if(!cool)
	{
		*buffer=S_CRK;
		*(buffer+1)=0;
		len=2;
		return len;
	}
	sprintf(cmd,"%08x/active",srv_ip);
	dir=opendir(cmd);
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",srv_ip,entry->d_name);
		f=fopen(cmd,"rb");
		fseek(f,1,SEEK_SET);
		fread(srv_opts,1,4,f);
		if((int)*srv_opts==(int)*crk_opts)
			cool=1;
		fclose(f);
		if(cool)
			break;
	}
	closedir(dir);
	if(cool)
	{
		printf("Cracked\n");
		*buffer=S_CRK;
		*(buffer+1)=1;
		memcpy(buffer+2,&srv_ip,4);
		len=6;
	}
	return len;
}
int create_software(unsigned char *buffer)
{
	int ip;
	char *fname = (char*)(buffer+10);
	char cmd[32];
	FILE*f;
	memset(cmd,0,32);
	memcpy(&ip,buffer+1,4);
	sprintf(cmd,"> %08x/",ip);
	memcpy(cmd+5,fname,strlen(fname));
	system(cmd);
	sprintf(cmd,"%08x/%s",ip,fname);
	f=fopen(cmd,"wb");
	if(f==NULL)
		return 0;
	if(fwrite(buffer+5,1,5,f)!=5)
	{
		fclose(f);
		return 0;
	}
	fclose(f);
	f=NULL;
	sprintf(cmd,"%08x/list",ip);
	f=fopen(cmd,"a");
	if(f==NULL)
		return 0;
	fprintf(f,"%s\n",fname);
	fclose(f);
	return 1;
}
int nmap(unsigned char * buffer)
{
	DIR*d=NULL;
	FILE*f;
	int len;
	unsigned char no=0;
	struct dirent *dir = NULL;
	char dir_name[16];
	char fname[64];
	int t_ip;
	memcpy(&t_ip,buffer+1,4);
	sprintf(dir_name,"%08x/active",t_ip);
	d=opendir(dir_name);
	memset(buffer,0,BSIZE);
	len=3;
	if(d==NULL)
	{
		printf("Error opening dir %08x/active\n",t_ip);
		*(buffer+1)=0;
		len=2;
	}
	else
		while((dir=readdir(d))!=NULL)
		{
			printf("%s\n",dir->d_name);
			if(!strcmp(".",dir->d_name)||!strcmp("..",dir->d_name))
				continue;
			sprintf(fname,"%08x/active/%s",t_ip,dir->d_name);
			f=fopen(fname,"rb");
			if(f==NULL)
				continue;
			fseek(f,1,SEEK_SET);
			fread(buffer+len++,1,1,f);
			no++;
			strcpy((char*)(buffer+len),dir->d_name);
			len+=16;
			fclose(f);
			f=NULL;
		}
	*buffer=S_RCN;
	*(buffer+1)=1;
	*(buffer+2)=no;
	closedir(d);
	return len;
}
int check_file_exists(unsigned char * buffer)
{
	int len,ip,err;
	char c;
	char fname[32];
	memset(fname, 0, 32);
	memcpy(&ip,buffer+1,4);
	sprintf(fname,"%08x/%s",ip,(char*)(buffer+5));
	err = access(fname,F_OK);
	c=err==-1?0:1;
	*buffer = S_CFE;
	*(buffer+1)=c;
	len = 2;
	return len;
}
int cmd_exec(unsigned char*buffer)
{
	int len;
	switch(*(buffer+1))
	{
		case LS:
			len = list(buffer);
			break;
		case START:
			len = start_p(buffer);
			break;
		case KILL:
			len = kill_p(buffer);
			break;
		case RM:
			len = remove_file(buffer);
			break;
		case FUZZ:
			len = fuzz(buffer);
			break;
		case CRACK:
			len = crack_otg(buffer);
			break;
		default:
			len = -1;
			break;
	}
	return len;
}
int crack_otg(unsigned char*buffer)
{
	int len,a_ip,t_ip;
	char opts[4],cmd[64],serv[4];
	char port;
	FILE*f=NULL;
	DIR*dir=NULL;
	struct dirent *entry=NULL;
	memcpy(&a_ip,buffer+2,4);
	memcpy(&t_ip,buffer+6,4);
	memcpy(opts,buffer+10,4);
	memset(buffer,0,BSIZE);
	sprintf(cmd,"%08x/active",a_ip);
	dir=opendir(cmd);
	printf("DEBUG STEP 1\n");
	if(dir==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	printf("DEBUG STEP 1.1\n");
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",a_ip,entry->d_name);
		f=fopen(cmd,"rb");
		fseek(f,1,SEEK_SET);
		fread(&port,1,1,f);
		if(port==opts[0])
			break;
		fclose(f);
		f=NULL;
	}
	printf("DEBUG STEP 2\n");
	closedir(dir);
	entry=NULL;
	if(f==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	fclose(f);
	sprintf(cmd,"%08x/active",t_ip);
	dir=opendir(cmd);
	if(dir==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	printf("DEBUG STEP 3\n");
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",t_ip,entry->d_name);
		f=fopen(cmd,"rb");
		fseek(f,1,SEEK_SET);
		fread(serv,1,4,f);
		if(serv[0]==port)
			break;
		fclose(f);
		f=NULL;
	}
	fclose(f);
	closedir(dir);
	if(f==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	printf("DEBUG STEP 4\n");
	if(opts[1]==serv[1]&&opts[2]==serv[2]&&opts[3]==serv[3])
	{
		len=6;
		*buffer=S_CMD;
		*(buffer+1)=1;
		memcpy(buffer+2,&t_ip,4);
	}
	else
	{
		len=2;
		*buffer=S_CMD;
		*(buffer+1)=0;
	}
	return len;
}
int fuzz(unsigned char*buffer)
{
	int len,t_ip,o_ip;
	int r,i,j,hlp[3];
	FILE*f;
	DIR*d;
	struct dirent*entry=NULL;
	char cmd[64];
	unsigned char port,h,exp[3];
	memcpy(&o_ip,buffer+2,4);
	memcpy(&t_ip,buffer+6,4);
	port=*(buffer+10);
	memset(cmd,0,64);
	memset(buffer,0,BSIZE);
	sprintf(cmd,"%08x/active",t_ip);
	d=opendir(cmd);
	if(d==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	h=0;
	while((entry=readdir(d))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",t_ip,entry->d_name);
		f=fopen(cmd,"rb");
		if(f==NULL)
			continue;
		fseek(f,1,SEEK_SET);
		fread(&h,1,1,f);
		if(h==port)
			break;
		else
		{
			h=0;
			fclose(f);
		}
	}
	closedir(d);
	if(!h)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	fread(exp,1,3,f);
	fclose(f);
	srand((unsigned int)time(NULL));
	for(i=0;i<8;i++)
		*(buffer+2+i)=(unsigned char)((rand()%26)+0x61);
	hlp[0]=(int)(rand()%3);
	do
		hlp[1]=(int)(rand()%6);
	while(hlp[1]<=hlp[0]);
	do
		hlp[2]=(int)(rand()%8);
	while(hlp[2]<=hlp[1]);
	*(buffer+2+hlp[0])=code_to_char(exp[0]);
	*(buffer+2+hlp[1])=code_to_char(exp[1]);
	*(buffer+3+hlp[2])=code_to_char(exp[2]);
	len=10;
	*buffer=S_CMD;
	*(buffer+1)=1;
	return len;
}
unsigned char code_to_char(unsigned char c)
{
	unsigned char out;
	switch(c)
	{
		case DOS:out='d';break;
		case INJ:out='i';break;
		case BOF:out='b';break;
		case SPOOF:out='s';break;
		case MITM:out='m';break;
		case EOP:out='e';break;
	}
	return out;
}
int remove_file(unsigned char*buffer)
{
	int len,err;
	struct dirent*entry;
	int o_ip,c_ip;
	char cmd[64];
	memcpy(&o_ip,buffer+2,4);
	memcpy(&c_ip,buffer+6,4);
	sprintf(cmd,"%08x/%s",c_ip,(char*)(buffer+10));
	if(access(cmd,F_OK)==-1)
	{
		len=2;
		memset(buffer,0,BSIZE);
		*buffer=S_CMD;
		return len;
	}
	sprintf(cmd,"rm %08x/%s",c_ip,(char*)(buffer+10));
	system(cmd);
	memset(buffer,0,BSIZE);
	*buffer=S_CMD;
	*(buffer+1)=1;
	len=2;
	return len;
}
int auth(unsigned char *buffer)
{
	int ip,c,err,i;
	char*user=(char*)(buffer+1);
	char*pass=(char*)(buffer+17);
	char u[16];
	char p[16];
	FILE*authf=NULL;
	printf("Authenticating\n%s\t%s\n",user,pass);
	authf=fopen(LOGINF,"rb");
	if(authf==NULL)
	{
		printf("Error opening file\n");
		return -1;
	}
	err=fread(&c,4,1,authf);
	err=0;
	for(i=0;i<c;i++)
	{
		memset(u,0,16);
		memset(p,0,16);
		fread(u,1,16,authf);
		fread(p,1,16,authf);
		fread(&ip,4,1,authf);
		printf("%s\t%s\n",u,p);
		if(!strcmp(u,user)&&!strcmp(p,pass))
		{
			err=1;
			break;
		}
	}
	*buffer=S_AUTH;
	*(buffer+1)=err==1?1:0;
	ip=err==1?ip:0;
	memcpy(buffer+2,&ip,4);
	return 6;
}
int start_p(unsigned char * buffer)
{
	int ip,len;
	unsigned char serv,act;
	char cmd[64];
	DIR*dir;
	struct dirent *entry;
	FILE*f;
	memcpy(&ip,buffer+6,4);
	sprintf(cmd,"%08x/%s",ip,(char*)(buffer+10));
	f=fopen(cmd,"rb");
	if(f==NULL)
	{
		len=2;
		memset(buffer,0,BSIZE);
		*buffer=S_CMD;
		*(buffer+1)=0;
		return len;
	}
	fseek(f,1,SEEK_SET);
	fread(&serv,1,1,f);
	fclose(f);
	sprintf(cmd,"%08x/active",ip);
	dir=opendir(cmd);
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))
			continue;
		sprintf(cmd,"%08x/active/%s",ip,entry->d_name);
		f=fopen(cmd,"rb");
		if(f==NULL)
		{
			len=2;
			memset(buffer,0,BSIZE);
			*buffer=S_CMD;
			*(buffer+1)=0;
			closedir(dir);
			return len;
		}
		fseek(f,1,SEEK_SET);
		fread(&act,1,1,f);
		fclose(f);
		if(act==serv)
		{
			sprintf(cmd,"rm %08x/active/%s",ip,entry->d_name);
			system(cmd);
			break;
		}
	}
	closedir(dir);
	sprintf(cmd,"cp %08x/%s %08x/active/",ip,(char*)(buffer+10),ip);
	system(cmd);
	memset(buffer,0,BSIZE);
	*buffer=S_CMD;
	*(buffer+1)=1;
	len=2;
	return len;
}
int kill_p(unsigned char*buffer)
{
	int ip,len;
	char cmd[64];
	memcpy(&ip,buffer+6,4);
	sprintf(cmd,"%08x/active/%s",ip,(char*)(buffer+10));
	if(access(cmd,F_OK)==-1)
	{
		memset(buffer,0,BSIZE);
		*buffer=S_CMD;
		*(buffer+1)=0;
		len=2;
		return len;
	}
	sprintf(cmd,"rm %08x/active/%s",ip,(char*)(buffer+10));
	system(cmd);
	memset(buffer,0,BSIZE);
	*buffer=S_CMD;
	*(buffer+1)=1;
	len=2;
	return len;
}
int list(unsigned char *buffer)
{
	int len=0,ip;
	char c,fname[16];
	DIR*dir;
	struct dirent*entry;
	memcpy(&ip,buffer+6,4);
	memset(buffer,0,BSIZE);
	sprintf(fname,"%08x",ip);
	dir=opendir(fname);
	if(dir==NULL)
	{
		len=2;
		*buffer=S_CMD;
		return len;
	}
	len = 2;
	*buffer=S_CMD;
	*(buffer+1)=1;
	while((entry=readdir(dir))!=NULL)
	{
		if(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name)||!strcmp("active",entry->d_name)||!strcmp("list",entry->d_name))
			continue;
		strcpy((char*)(buffer+len),entry->d_name);
		len+=strlen(entry->d_name);
		*(buffer+len)='\n';
		len++;
	}
	closedir(dir);
	return len;
}
void check_error(int e)
{
	switch(e)
	{
		case EBADF:
			printf("Not a valid File Descriptor\n");
			break;
		case EFAULT:
			printf("Not a valid part on optval or optlen\n");
			break;
		case EINVAL:
			printf("optlen invalid\n");
			break;
		case ENOPROTOOPT:
			printf("Unknown option\n");
			break;
		case ENOTSOCK:
			printf("its a file, not a socket\n");
			break;
		default:
			printf("Wot\n");
	}
}
