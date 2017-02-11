/*
 Services and Cracks Creation
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "header.h"

int create_lobby(unsigned char* buffer)
{
	int opt,len;
	system("clear");
	printf("Welcome to the interactive Software Creation\n");
	printf("Choose what type of software do you want to create\n");
	printf("\t[1] Service\n\t[2] Cracker\n");
	printf(">> ");
	opt=read_int(1,2);
	len=6;
	if(opt==1)
	{
		*(buffer+5)=SERVICE;
		len+=create_service(buffer);
	}
	else
	{
		*(buffer+5)=CRACKER;
		len+=create_cracker(buffer);
	}
	printf("Now it's time to give it a name\n");
	scanf("%15s",(char*)(buffer+10));
	clean_stdin();
	len+=16;
	return len;
}
int read_int(int min,int max)
{
	int ret,cnt;
	do
	{
		cnt=1;
		scanf("%d",&ret);
		clean_stdin();
		if(ret<min||ret>max)
		{
			cnt=0;
			printf("Invalid Option\n");
		}
	}
	while(!cnt);
	return ret;
}
int create_service(unsigned char*buffer)
{
	int len=0,i,opt,r;
	//unsigned int t=1000;
	printf("\nSo you want to create a service\n");
	printf("Please choose what type of service it shall be\n");
	printf("\t[1] FTP\n\t[2] SSH\n");
	opt=read_int(1,2);
	*(buffer+6)=opt==1?FTP:SSH;
	len++;
	printf("You can't control the creation of the service\n");
	printf("Press any key to continue...");
	getchar();
	printf("Compiling program...");
	//while((t=sleep(t))>100);
	printf("Your software vulnerabilities will be:\n");
	srand((unsigned int)time(NULL));
	for(i=0;i<3;i++)
	{
		r=(rand()%6)+1;
		switch(r)
		{
			case 1:
				printf("\tDenial of Service\n");
				*(buffer+7+i)=DOS;
				break;
			case 2:
				printf("\tCode Injection\n");
				*(buffer+7+i)=INJ;
				break;
			case 3:
				printf("\tBuffer overflow\n");
				*(buffer+7+i)=BOF;
				break;
			case 4:
				printf("\tSpoofing\n");
				*(buffer+7+i)=SPOOF;
				break;
			case 5:
				printf("\tMan in the Middle Attack\n");
				*(buffer+7+i)=MITM;
				break;
			case 6:
				printf("\tElevation of Provileges\n");
				*(buffer+7+i)=EOP;
				break;
			default:
				// dude
				printf("\tBuffer overflow\n");
				*(buffer+7+i)=BOF;
				break;
		}
		len++;
	}
	return len;
}
int create_cracker(unsigned char*buffer)
{
	int len=0,i,opt;
	printf("\nSo you want to create a cracker\n");
	printf("I will assume you know how to create a cracker...\n");
	printf("First of all, choose which service you want to attack\n");
	printf("\t[1] FTP\n\t[2] SSH\n");
	opt=read_int(1,2);
	*(buffer+6)=opt==1?FTP:SSH;
	len++;
	printf("Okay then, now...\n");
	printf("There are three phases of an attack / crack\n");
	printf("You will choose each one of them from this list\n");
	printf("\t[1] Denial of Service\n");
	printf("\t[2] Code Injection\n");
	printf("\t[3] Buffer Overflow\n");
	printf("\t[4] Spoof\n");
	printf("\t[5] Man in the Middle attack\n");
	printf("\t[6] Elevation of Privilege\n");
	for(i=0;i<3;i++)
	{
		if(!i)
			printf("First one\n>> ");
		if(i==1)
			printf("Second one\n>> ");
		if(i==2)
			printf("Last one\n>> ");
		opt=read_int(1,6);
		switch(opt)
		{
			case 1:
				*(buffer+7+i)=DOS;
				break;
			case 2:
				*(buffer+7+i)=INJ;
				break;
			case 3:
				*(buffer+7+i)=BOF;
				break;
			case 4:
				*(buffer+7+i)=SPOOF;
				break;
			case 5:
				*(buffer+7+i)=MITM;
				break;
			case 6:
				*(buffer+7+i)=EOP;
				break;
			default:
				//Dude what?
				*(buffer+7+i)=BOF;
				break;
		}
		len++;
	}
	printf("Good good ...\n");
	return len;
}
