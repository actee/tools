#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Usage: %s <port[s]> [<path_to_file>]\n",argv[0]);
		return 1;
	}
	char cmd[128];
	if(argc==3 && atoi(argv[(argc-1)]) == 0)
	{
		cmd[128];
		sprintf(cmd,"grep \"  %s  \" %s",argv[1],argv[2]);
		system(cmd);
	}
	if(argc==2)
	{
		if(atoi(argv[1]) == 0)
		{
			printf("Please specify a valid port\n");
			return 1;
		}
		cmd[128];
		sprintf(cmd,"grep \"  %s  \" /media/tiateixas/NYX/RFCs/port_numbers.txt",argv[1]);
		system(cmd);
	}
	if(argc > 3 && atoi(argv[(argc-1)])==0)
	{
		int i=1;
		for(i=1;i<(argc-1);i++)
		{
			sprintf(cmd,"grep \"  %s  \" %s",argv[i],argv[(argc-1)]);
			system(cmd);
		}
	}
	if(argc >= 3 && atoi(argv[(argc-1)]) != 0)
	{
		int i;
		for(i=1;i<argc;i++)
		{
			sprintf(cmd,"grep \"  %s  \" /media/tiateixas/NYX/RFCs/port_numbers.txt",argv[i]);
			system(cmd);
		}
	}
	return 0;
}
