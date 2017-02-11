/*
 Command parser for hgame
 
 Copyright 2017 Tiago Teixeira
 This file is part of hgameOS
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "header.h"

int parse_cmd(struct cmd* args,char*helper)
{
	char *c,cur,s_cmd[128],*m_cmd;
	char*opts[3];
	opts[0]=NULL;
	opts[1]=NULL;
	opts[2]=NULL;
	int op,cnt=0;
	memset(s_cmd,0,128);
	m_cmd=s_cmd;
	// read char by char
	do
	{
		cur=getchar();
		if(cur!='\n'&&cur!=8)
			s_cmd[cnt++]=cur;
		if(cur==8)
		{
			s_cmd[cnt]=0;
			cnt--;
		}
	}
	while(cur!='\n');
	cnt=0;
	c=s_cmd;
	while(*c!='\0')
	{
		if(*c==' ')
		{
			c++;
			*(c-1)='\0';
			opts[cnt]=c;
			cnt+=1;
		}
		else
			c++;
	}
	if(!strcmp("connect",m_cmd))
	{
		op=CONNECT;
		if(!strcmp("localhost",opts[0]))
		   args->cmd_opt1 = LCLHOST;
		else if(inet_addr(opts[0])!=-1)
			args->cmd_opt1 = inet_addr(opts[0]);
		else
		{
			printf("Not a valid IP\n");
			args->cmd_opt1=NULL_IP;
		}
	}
	else if(!strcmp("logout",m_cmd))
		op=LOGOUT;
	else if(!strcmp("poweroff",m_cmd))
		op=POWOFF;
	else if(!strcmp("help",m_cmd))
		op=HELP;
	else if(!strcmp("clear",m_cmd))
		op=CLEAR;
	else if(!strcmp("ls",m_cmd))
		op=LS;
	else if(!strcmp("scan",m_cmd)||!strcmp("nmap",m_cmd))
		op=NMAP;
	else if(!strcmp("create",m_cmd))
		op=CREAT;
	else if(!strcmp("start",m_cmd))
	{
		if(opts[0]==NULL)
			op=ERROR;
		else
		{
			strcpy(helper,opts[0]);
			op=START;
		}
	}
	else if(!strcmp("kill",m_cmd))
	{
		if(opts[0]==NULL)
			op=ERROR;
		else
		{
			strcpy(helper,opts[0]);
			op=KILL;
		}
	}
	else if(!strcmp("upload",m_cmd)||!strcmp("ul",m_cmd))
	{
		if(opts[0]==NULL)
			op=ERROR;
		else
		{
			memset(helper,0,64);
			strcpy(helper,opts[0]);
			op=UPLD;
		}
	}
	else if(!strcmp("download",m_cmd)||!strcmp("dl",m_cmd))
	{
		if(opts[0]==NULL)
			op=ERROR;
		else
		{
			strcpy(helper,opts[0]);
			op=DNLD;
		}
	}
	else if(!strcmp("rm",m_cmd))
	{
		if(opts[0]==NULL)
			op=ERROR;
		else
		{
			strcpy(helper,opts[0]);
			op=RM;
		}
	}
	else if(!strcmp("fuzz",m_cmd))
	{
		if(atoi(opts[0]))
		{
			if(atoi(opts[0])==21)
				args->cmd_opt1=FTP;
			else
				args->cmd_opt1=SSH;
			op=FUZZ;
		}
		else
			op=ERROR;
	}
	else if(!strcmp("crack",m_cmd))
	{
		if(atoi(opts[0])&&opts[1]!=NULL)
		{
			args->cmd_opt1=atoi(opts[0]);
			strcpy(helper,opts[1]);
			op=CRACK;
		}
		else
			op=ERROR;
	}
	else if(!strcmp("active",m_cmd))
		op=ACTIVE;
	else
	{
		op=ERROR;
		strcpy(helper,m_cmd);
	}
	return op;
}
void parse_nmap(unsigned char *buffer)
{
	unsigned char i;
	int off=2;
	printf("Scan complete!\n");
	printf("Found %hhu ports open\n",*(buffer+2));
	for(i=0;i<*(buffer+2);i++)
	{
		if(*(buffer+(++off))==FTP)
		{
			off++;
			printf("\tFTP on port 21\t%s\n",(char*)(buffer+off));
			off+=16;
		}
		else if(*(buffer+(++off))==SSH)
		{
			off++;
			printf("\tFTP on port 22\t%s\n",(char*)(buffer+off));
			off+=16;
		}
	}
}
void clean_stdin(void)
{
	char c;
	while((c=getchar())!='\n'&&c!=EOF);
}
