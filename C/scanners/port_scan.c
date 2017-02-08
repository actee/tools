/*
	C Syn Port Scan
	Ack optional to add later
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
/*#include <netinet/ip_icmp.h>*/
#include <arpa/inet.h>
#include <pthread.h> // Threads

#define SEQ_NUM 1105024978

void *receive_ack(void *ptr);  //For threads
void *receive_icmp(void *ptr); //For threads
int icmp_sniffer();
int start_sniffer();
void process_icmp(unsigned char *buffer, int size);
void process_packet(unsigned char *buffer, int size);
int get_local_ip(char *);
unsigned short csum(unsigned short *ptr, int nbytes);

// For checksum
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

// ICMP STUFF --------------
struct icmp {
	u_char icmp_type;
	u_char icmp_code;
	u_short icmp_cksum;
	u_int32_t icmp_unused;
};
//Original 64-bit TCP datagram (ports)
struct ori_dg_tcp {
	short sport;
	short dport;
	u_int32_t seq;
};
// END ICMP STUFF ----------

int main (int argc, char *argv[])
{
	//Testing - TODO Add user interface to it
	// code: freopen("/tmp/out.txt","w",stdout);
	//END TESTING
	
	//Deb
	printf("Seq_num %d\n",SEQ_NUM);
	
	if(argc < 3){printf("Specify ports\n");exit(0);}
	
	//Save to file
	char fn[256];
	sprintf(fn,"> /tmp/ports_%s",argv[1]);
	system(fn);
	sprintf(fn,"ports=/tmp/ports_%s",argv[1]);
	putenv(fn);
	printf("File with ports on /tmp/ports_%s or in $ports\n",argv[1]);
	
	//raw socket
	int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	
	if(s==-1)
	{
		// socket failure
		perror("Failed to create socket\n");
		exit(1);
	} else {
		printf("Socket created\n");
	}
	
	// datagram to represent packet
	char datagram[4096];
	
	// zero out packet
	memset(datagram, 0, 4096);
	
	// IP HEader
	struct iphdr *iph = (struct iphdr *) datagram;
	
	// TCP Header
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
	
	struct sockaddr_in dest;
	struct pseudo_header psh;
	
	char *target = argv[1];
	
	if (inet_addr(target) != -1)
	{
		dest_ip.s_addr = inet_addr(target);
	}
	else
	{
		printf("Please specify a valid IP address\n");
		exit(1);
	}
	
	int source_port = 43591;
	char source_ip[20];
	get_local_ip(source_ip);
	
	printf("Local source IP s %s\n", source_ip);
	
	// Fill IP header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct ip)+sizeof(struct tcphdr);
	iph->id = htons(54321); // Id of packet
	iph->frag_off = htons(16384);
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0; // Checksum later
	iph->saddr = inet_addr(source_ip);
	iph->daddr = dest_ip.s_addr;
	
	iph->check = csum((unsigned short *) datagram, iph->tot_len>>1);
	
	//TCP Header
	tcph->source = htons(source_port);
	tcph->dest = htons(80);
	tcph->seq = htonl(SEQ_NUM);
	tcph->ack_seq=0;
	tcph->doff = sizeof(struct tcphdr) / 4;
	tcph->fin=0;
	tcph->syn=1;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
	tcph->window = htons(14600);
	tcph->check = 0; // Kernel's job to fill
	tcph->urg_ptr = 0;
	
	// Turn to IP_RAW
	int one = 1;
	const int *val = &one;
	if(setsockopt(s,IPPROTO_IP,IP_HDRINCL,val,sizeof(one)) < 0)
	{
		printf("Error setting IP_HDRINCL\n");
		exit(0);
	}
	
	//Sniffer thread
	char *message1 = "Thread 1";
	int iret1;
	pthread_t sniffer;
	if(pthread_create(&sniffer,NULL,receive_ack,(void *)message1) < 0)
	{
		printf("Thread Failure (1).\n");
		exit(0);
	}
	//ICMP sniffer thread
	char *message2 = "Thread 2";
	int iret2;
	pthread_t icmp_sniffer;
	if(pthread_create(&icmp_sniffer,NULL,receive_icmp,(void *)message2) < 0)
	{
		printf("Thread Failure (2).\n");
		exit(0);
	}
	
	
	int i;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	for(i=2;i<argc;i++)
	{
		tcph->dest = htons(atoi(argv[i]));
		tcph->check=0;
		psh.source_addr = inet_addr(source_ip);
		psh.dest_addr = dest.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_TCP;
		psh.tcp_length = htons(sizeof(struct tcphdr));
		
		memcpy(&psh.tcp , tcph, sizeof(struct tcphdr));
		
		tcph->check = csum((unsigned short *)&psh,sizeof(struct pseudo_header));
		// Send packet
		if(sendto(s,datagram,sizeof(struct iphdr)+sizeof(struct tcphdr),0,(struct sockaddr *)&dest, sizeof(dest)) < 0)
		{
			printf("Error sending syn packet.\n");
			exit(0);
		}
		printf("Sent packet to port %d\n",atoi(argv[i]));
		printf("DEBUG - tcp check is %hu\n",tcph->check);
	}
	
	pthread_join(sniffer,NULL);
	pthread_join(icmp_sniffer,NULL);
	printf("%d",iret1);
	printf("%d",iret2);
	
	return 0;
}

void * receive_ack(void *ptr)
{
	start_sniffer();
}
void * receive_icmp(void *ptr)
{
	icmp_sniffer();
}

int start_sniffer()
{
	int sock_raw;
	
	int saddr_size, data_size;
	struct sockaddr saddr;
	
	unsigned char *buffer = (unsigned char*)malloc(65536);
	
	printf("Sniffer running...\n");
	fflush(stdout);
	
	//raw socket to sniff
	sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if(sock_raw<0)
	{
		printf("Socket Error\n");
		fflush(stdout);
		return 1;
	}
	
	saddr_size = sizeof saddr;
	while(1)
	{
		//Receive packet
		data_size = recvfrom(sock_raw, buffer, 65536,0, &saddr, &saddr_size );
		if(data_size < 0)
		{
			printf("recvfrom error");
			fflush(stdout);
			return 1;
		}
		//process packet
		process_packet(buffer,data_size);
	}
	
	close(sock_raw);
	printf("Sniffer finished\n");
	fflush(stdout);
	return 0;
}

int icmp_sniffer()
{
	int icmp_socket;
	int saddr_size,data_size;
	struct sockaddr saddr;
	
	unsigned char *buffer = (unsigned char *)malloc(65536);
	
	printf("ICMP Sniffer runnning...\n");
	fflush(stdout);
	
	//socket init
	icmp_socket = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	if(icmp_socket < 0 )
	{
		printf("Socket Error (ICMP)\n");
		fflush(stdout);
		return 1;
	}
	
	saddr_size = sizeof saddr;
	while(1)
	{
		//Receive ICMP Packet
		data_size = recvfrom(icmp_socket,buffer, 65536,0,&saddr, &saddr_size);
		if(data_size < 0)
		{
			printf("recvfrom error (icmp)\n");
			fflush(stdout);
			return 1;
		}
		//printf("ICMP packet received\n");
		//fflush(stdout);
		//process icmp packet
		process_icmp(buffer, data_size);
	}
	
	close(icmp_socket);
	printf("Sniffer finished (ICMP)\n");
	fflush(stdout);
	return 0;
}

void process_icmp(unsigned char *buffer, int size)
{
	struct iphdr *iph = (struct iphdr*)buffer;
	struct sockaddr_in source, dest;
	unsigned short iphdrlen;
	unsigned short icmphdrlen = 8;
	
	if(iph->protocol == 1) //for ICMP
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl*4;
		
		memset(&source,0,sizeof(source));
		source.sin_addr.s_addr = iph->saddr;
		
		struct icmp *icmph = (struct icmp *)(buffer + iphdrlen);
		
		//printf("ICMP type %d code %d\n",icmph->icmp_type,icmph->icmp_code);
		//fflush(stdout);
		
		if(icmph->icmp_type == 3 /*&& icmph->icmp_code == 3*/ && source.sin_addr.s_addr == dest_ip.s_addr)
		{
			struct iphdr *iphb = (struct iphdr *)(buffer+iphdrlen+8);
			unsigned short iphdrlenb = iphb->ihl*4;
			struct ori_dg_tcp *ports = (struct ori_dg_tcp *)(buffer+iphdrlen+iphdrlenb+icmphdrlen); // 8 = sizeof icmp header
			printf("Port %d filtered\t(type %d code %d)\n",ntohs(ports->dport),icmph->icmp_type,icmph->icmp_code);
			fflush(stdout);
		}
		/*
		if(icmph->icmp_type == 3)
		{
			//printf("ICMP type 3\n");
			switch(icmph->icmp_code)
			{
				case 1:
					printf("Code 1 - Destination host unreachable\n");
					break;
				case 2:
					printf("Code 2 - Destination protocol unreachable\n");
					break;
				case 4:
					printf("Code 4 - Fragmentation Required\n");
					break;
				case 5:
					printf("Code 5 - Source route failed\n");
					break;
				case 6:
					printf("Code 6 - Destination network unknown\n");
					break;
				case 7:
					printf("Code 7 - Destination host unknown\n");
					break;
				case 8:
					printf("Code 8 - Source host isolated\n");
					break;
				case 9:
					printf("Code 9 - Network administratively prohibited\n");
					break;
				case 10:
					printf("Code 10 - Host administratively prohibited\n");
					break;
				case 11:
					printf("Code 11 - Network unreachable for ToS\n");
					break;
				case 12:
					printf("Code 12 - Host unreachable for ToS\n");
					break;
				case 13:
					printf("Code 13 - Communication administratively prohibited\n");
					break;
				case 14:
					printf("Code 14 - Host precedence violation\n");
					break;
				case 15:
					printf("Code 15 - Precedence cutoff in effect\n");
					break;
				default:
					printf("Code %d\n",icmph->icmp_code);
					break;
			}
		}
		*/
	}
}
		

void process_packet(unsigned char *buffer, int size)
{
	struct iphdr *iph = (struct iphdr*)buffer;
	struct sockaddr_in source, dest;
	unsigned short iphdrlen;
	unsigned short tcphdrlen;
	char fn[64];
	
	if(iph->protocol == 6)
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl*4;
		
		struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen);
		
		memset(&source,0,sizeof(source));
		source.sin_addr.s_addr = iph->saddr;
		
		memset(&dest,0,sizeof(dest));
		dest.sin_addr.s_addr = iph->daddr;
		
		if(tcph->syn == 1 && tcph->ack== 1 && source.sin_addr.s_addr == dest_ip.s_addr)
		{
			sprintf(fn,"echo \"Port %d open\" >> $ports",ntohs(tcph->source));
			printf("Port %d open\n",ntohs(tcph->source));
			fflush(stdout);
			system(fn);
		}
		else if(tcph->rst == 1 && source.sin_addr.s_addr == dest_ip.s_addr)
		{
			sprintf(fn,"echo \"Port %d closed\" >> $ports",ntohs(tcph->source));
			printf("Port %d is closed\n",ntohs(tcph->source));
			fflush(stdout);
			system(fn);
		}
	}
}

int get_local_ip(char *buffer)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	
	const char *kGoogleDnsIp = "8.8.8.8";
	int dns_port = 53;
	
	struct sockaddr_in serv;
	
	memset(&serv, 0, sizeof(serv));
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=inet_addr(kGoogleDnsIp);
	serv.sin_port = htons(dns_port);
	
	int err = connect(sock, (const struct sockaddr *)&serv, sizeof(serv));
	
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock,(struct sockaddr*)&name,&namelen);
	
	const char *p = inet_ntop(AF_INET,&name.sin_addr, buffer, 100);
	
	close(sock);
}

unsigned short csum(unsigned short *ptr, int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;
	
	sum = 0;
	while(nbytes > 1)
	{
		sum += *ptr++;
		nbytes-=2;
	}
	if(nbytes==1)
	{
		oddbyte = 0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum += oddbyte;
	}
	
	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer = (short)~sum;
	
	return(answer);
}

