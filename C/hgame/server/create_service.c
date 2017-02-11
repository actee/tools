/*
 Server side service creation

 char	- service
 char	- port
 char*3	- options
 
 Copyright 2017 Tiago Teixeira
 This file is part of hgameOS
*/
#include <stdio.h>
#include <string.h>

char check_valid(char c)
{
	char opt;
	switch(c)
	{
		case 'd':
			opt=31;
			break;
		case 'i':
			opt=32;
			break;
		case 'b':
			opt=33;
			break;
		case 's':
			opt=34;
			break;
		case 'm':
			opt=35;
			break;
		case 'e':
			opt=36;
			break;
		default:
			opt=-1;
	}
	return opt;
}
int main(int argc, char**argv)
{
	if(argc < 5)
	{
		printf("Usage: %s <port> <3 opts>\n",argv[0]);
		printf("Valid options: d i b s m e\n");
		return 1;
	}
	char o,s,a,b,c;
	int se;
	char name[16];
	FILE*f=NULL;
	if(!(se=atoi(argv[1])))
	{
		printf("Use a valid port ( 22 / 21 )\n");
		return 1;
	}
	memset(name,0,16);
	s=(char)se;
	a=*(argv[2]);
	b=*(argv[3]);
	c=*(argv[4]);
	a=check_valid(a);
	b=check_valid(b);
	c=check_valid(c);
	if(a==-1||b==-1||c==-1)
	{
		printf("Invalid options\n");
		return 1;
	}
	if(s!=21&&s!=22)
	{
		printf("Invalid service\n");
		return 1;
	}
	printf("Specify name of software\n>> ");
	scanf("%16s",name);
	f=fopen(name,"wb");
	if(f==NULL)
	{
		printf("Could not create file\n");
		return 1;
	}
	o=12;
	fwrite(&o,1,1,f);
	fwrite(&s,1,1,f);
	fwrite(&a,1,1,f);
	fwrite(&b,1,1,f);
	fwrite(&c,1,1,f);
	fclose(f);
	printf("Software created cool\n");
	return 0;
}
