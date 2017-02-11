/*
*
* Create NPC server for hgame
*
*/
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char**argv)
{
	if(argc<2)
	{
		printf("Usage: %s <ip>\n",argv[0]);
		return 1;
	}
	int ip;
	if((ip=inet_addr(argv[1]))==-1)
	{
		printf("Invalid IP\n");
		return 1;
	}
	char cmd[64];
	FILE*f;
	f=fopen("ip.list","a");
	fprintf(f,"\t%s\n",argv[1]);
	fclose(f);
	sprintf(cmd, "mkdir %08x;mkdir %08x/active",ip,ip);
	system(cmd);
	printf("Good\n");
	return 0;
}
