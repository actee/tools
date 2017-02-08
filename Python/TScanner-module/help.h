/*
 Header Checksum
*/
#include <stdio.h>
#ifndef HELP_H
#define HELP_H
struct flags_tw
{
	short port;
	u_int8_t flags;
	/*
	one bit for each
	*/
};
struct sniffer_info
{
	int socket;
	unsigned int ip;
	FILE *fd;
	char *buffer;
};
struct pseudo_header
{
	u_int32_t source_addr;
	u_int32_t dest_addr;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
	struct tcphdr tcp;
};
unsigned short csum(unsigned short *ptr,int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	short answer;
	
	sum = 0;
	while(nbytes>1)
	{
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1)
	{
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=sum>>16;
	answer=(short)~sum;
	return(answer);
}
#endif