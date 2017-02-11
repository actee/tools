/*
 Register user
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define LOGINF	"logins.bin"
int main(void)
{
	char user[16];
	char pass[16];
	char u[16];
	char p[16];
	char sip[16];
	char cmd[32];
	int err,ip,i,cnt;
	FILE*authf;
	int c;
	if(access(LOGINF,F_OK)==-1)
	{
		authf=fopen(LOGINF,"wb");
		if(authf==NULL)
		{
			printf("Can't create file\n");
			return 1;
		}
		c=0;
		if(fwrite(&c,4,1,authf)!=1)
		{
			printf("Couldn't write to file\n");
			return 1;
		}
		fclose(authf);
		authf=NULL;
	}
	authf=fopen(LOGINF,"r+b");
	if(authf==NULL)
	{
		printf("Error opening file\n");
		return 1;
	}
	if(fread(&c,4,1,authf)!=1)
	{
		printf("Error reading from file\n");
		fclose(authf);
		return 1;
	}
	memset(user,0,16);
	memset(pass,0,16);
	printf("Enter username: ");
	scanf("%16s",user);
	printf("Enter password: ");
	scanf("%16s",pass);
	printf("Enter desired IP: ");
	scanf("%16s",sip);
	ip=inet_addr(sip);
	if(ip==-1)
	{
		printf("IP is invalid\n");
		fclose(authf);
		return 1;
	}
	for(cnt=0;cnt<c;cnt++)
	{
		fread(u,1,16,authf);
		fread(p,1,16,authf);
		fread(&i,4,1,authf);
		if(!strcmp(user,u))
		{
			printf("Username already taken\n");
			fclose(authf);
			return 0;
		}
		if(i==ip)
		{
			printf("IP already taken\n");
			fclose(authf);
			return 0;
		}
	}
	// All okay
	fseek(authf,(long)0,SEEK_END);
	fwrite(user,1,16,authf);
	fwrite(pass,1,16,authf);
	fwrite(&ip,4,1,authf);
	fseek(authf,(long)0,SEEK_SET);
	c+=1;
	sprintf(cmd,"mkdir %08x",ip);
	system(cmd);
	memset(cmd,0,32);
	sprintf(cmd,"mkdir %08x/active",ip);
	system(cmd);
	fwrite(&c,4,1,authf);
	printf("User registered with success\n");
	fclose(authf);
	return 0;
}
