/*
	C Ack Port Scan
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h> // Threads

#define SEQ_NUM 1105024978

void *thread_function(void *);
int icmp_sniffer();
void process_icmp();
int sniffer();
void process_packet(unsigned char *buffer, int size);
int get_local_ip(char *);
unsigned short csum(unsigned short *ptr, int nbytes);

// For csum
struct pseudo_header
{
	u_int32_t source_addr;
	u_int32_t dest_addr;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
	
	struct tcphdr tcp;
};
struct in_addr dest_ip;

struct icmphdr {
	unsigned char type;
	unsigned char code;
	unsigned short cksum;
	u_int32_t unused;
};

struct orhdr {
	u_short sport;
	u_short dport;
	u_int32_t seq;
};

int main(int argc, char *argv[])
{
	printf("Seq_num %d\n", SEQ_NUM);
	
	if(argc < 3){
		printf("Usage: %s <dest_ip> <ports>\n",argv[0]);
		exit(1);
	}
	
	int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	
	if (s==-1)
	{
		perror("Failed to create socket");
		exit(1);
	} else {
		printf("Socket created\n");
	}
	
	
	char datagram[4096];
	
	memset(datagram,0,4096);
	
	struct iphdr *iph = (struct iphdr *) datagram;
	struct tcphdr *tcph = (struct tcphdr *)(datagram + sizeof(struct ip));
	
	struct sockaddr_in dest;
	struct pseudo_header psh;
	
	char *target = argv[1];
	if(inet_addr(target) != -1){
		dest_ip.s_addr = inet_addr(target);
	}
	else
	{
		printf("Please specify a valid IP address\n");
		exit(1);
	}
	
	int source_port = 43518;
	char source_ip[20];
	get_local_ip(source_ip);
	
	printf("Local source IP is %s\n",source_ip);
	
	iph->ihl = 5;
	iph->version = 4;
	iph->tos=0;
	iph->tot_len = sizeof(struct ip)+sizeof(struct tcphdr);
	iph->id = htons(43555);
	iph->frag_off = htons(16384);
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0; //Fill later
	iph->saddr = inet_addr(source_ip);
	iph->daddr = dest_ip.s_addr;
	
	iph->check = csum((unsigned short*)datagram, iph->tot_len>>1);
	
	tcph->source = htons(source_port);
	tcph->dest = htons(80);
	tcph->seq = htonl(SEQ_NUM);
	tcph->ack = htons(456);
	tcph->doff = sizeof(struct tcphdr) / 4;
	tcph->fin=0;
	tcph->syn=0;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=1;
	tcph->urg=0;
	tcph->window = htons(14659);
	tcph->check = 0; //Kernel please
	tcph->urg_ptr = 0;
	
	int one = 1;
	const int *val = &one;
	if(setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
	{
		printf("Error setting IP_HDRINCL\n");
		exit(0);
	}
	
	int i;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	for(i=2;i<argc;i++)
	{
		tcph->dest = htons(atoi(argv[i]));
		tcph->check = 0;
		
		psh.source_addr = inet_addr(source_ip);
		psh.dest_addr = dest.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_TCP;
		psh.tcp_length = htons(sizeof(struct tcphdr));
		
		memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));
		
		tcph->check = csum((unsigned short *)&psh,sizeof(struct pseudo_header));
		
		//Send packet
		if(sendto(s,datagram,sizeof(struct iphdr)+sizeof(struct tcphdr),0,(struct sockaddr *)&dest, sizeof(dest)) < 0)
		{
			printf("Error sending packet\n");
			exit(0);
		}
		printf("Sent packet to port %d\n",atoi(argv[i]));
	}
	
	char *message1 = "Thread 1";
	int iret1;
	pthread_t sniffer_t;
	if(pthread_create(&sniffer_t,NULL,thread_function,(void *)message1) < 0)
	{
		printf("ICMP thread failure\n");
		exit(0);
	}
	
	sniffer();
	
	return 0;
}


int sniffer()
{
	int sock_raw;
	int saddr_size,data_size;
	struct sockaddr saddr;
	
	unsigned char *buffer = (unsigned char *)malloc(65536);
	
	printf("Starting sniffer...\n");
	
	sock_raw = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	if(sock_raw <0)
	{
		printf("Socket Error in sniffer()\n");
		return 1;
	}
	
	saddr_size = sizeof(saddr); // Or without parenthesis
	
	while(1)
	{
		//Receive
		data_size = recvfrom(sock_raw, buffer, 65536,0, &saddr, &saddr_size );
		if(data_size < 0)
		{
			printf("recvfrom() error\n");
			return 1;
		}
		//process
		process_packet(buffer,data_size);
	}
	
	close(sock_raw);
	printf("Sniffer finished\n");
	return 0;
}

void process_packet(unsigned char *buffer, int size)
{
	struct iphdr *iph = (struct iphdr *)buffer;
	struct sockaddr_in source, dest;
	unsigned short iphdrlen;
	
	if(iph->protocol == 6)
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl*4;
		
		struct tcphdr *tcph = (struct tcphdr *)(buffer+iphdrlen);
		
		memset(&source,0,sizeof(source));
		source.sin_addr.s_addr = iph->saddr;
		
		memset(&dest,0,sizeof(dest));
		dest.sin_addr.s_addr =  iph->daddr;
		
		if(tcph->rst==1 && source.sin_addr.s_addr == dest_ip.s_addr)
		{
			printf("Port %d unfiltered\n", ntohs(tcph->source));
		}
	}
}

void *thread_function(void *ptr)
{
	icmp_sniffer();
}

int icmp_sniffer()
{
	int sock_icmp;
	int saddr_size, data_size;
	struct sockaddr saddr;
	
	unsigned char *buffer = (unsigned char *)malloc(65536);
	
	printf("Initializing ICMP Sniffer...\n");
	fflush(stdout);
	
	sock_icmp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sock_icmp < 0)
	{
		printf("ICMP socket failure\n");
		fflush(stdout);
		return 1;
	}
	
	saddr_size = sizeof(saddr);
	while(1)
	{
		data_size = recvfrom(sock_icmp, buffer, 65536,0,&saddr,&saddr_size);
		if(data_size < 0)
		{
			printf("recv_from failure\n");
			fflush(stdout);
			return 1;
		}
		process_icmp(buffer, data_size);
	}
	close(sock_icmp);
	printf("ICMP sniffer finished\n");
	fflush(stdout);
	return 0;
}

void process_icmp(unsigned char *buffer, int size)
{
	struct iphdr *iph = (struct iphdr *)buffer;
	struct sockaddr_in source, dest;
	unsigned short iphdrlen;
	if(iph->protocol == 1)
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl * 4;
		
		struct icmphdr *icmp = (struct icmphdr *)(buffer + iphdrlen);
		
		memset(&source,0,sizeof(source));
		source.sin_addr.s_addr = iph->saddr;
		
		memset(&dest,0,sizeof(dest));
		dest.sin_addr.s_addr = iph->daddr;
		
		if(icmp->type== 3 && source.sin_addr.s_addr == dest_ip.s_addr)
		{
			struct orhdr *tcph = (struct orhdr *)(buffer+iphdrlen+8);
			printf("Port %d filtered\n",ntohs(tcph->sport));
			fflush(stdout);
		}
	}
}

int get_local_ip(char *buffer)
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	const char *kGoogleDnsIp = "8.8.8.8";
	int dns_port = 53;
	
	struct sockaddr_in serv;
	
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr=inet_addr(kGoogleDnsIp);
	serv.sin_port = htons(dns_port);
	
	int err = connect(sock,(const struct sockaddr *)&serv, sizeof(serv));
	
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	
	err = getsockname(sock,(struct sockaddr*)&name,&namelen);
	
	const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
	
	close(sock);
}

unsigned short csum(unsigned short *ptr , int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;
	
	sum = 0;
	while(nbytes>1)
	{
		sum += *ptr++;
		nbytes -= 2;
	}
	if(nbytes == 1)
	{
		oddbyte = 0;
		*((u_char *)&oddbyte)=*(u_char *)ptr;
		sum += oddbyte;
	}
	
	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer = (short)~sum;
	
	return(answer);
}
