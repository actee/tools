#include <Python.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

static PyObject *IPErr;
static PyObject *SendErr;

static PyObject * init_raw_socket(PyObject*self,PyObject*args)
{
	int socket = socket(PF_INET,SOCK_RAW,IPPROTO_TCP);
	if(socket==-1)
		return NULL;
	int one=1;
	const int *val=&one;
	if(setsockopt(socket,IPPROTO_IP,IP_HDRINCL,val,sizeof(one))<0)
		return NULL;
	return Py_BuildValue("i",socket);
}

static PyObject * get_local_ip(PyObject*self,PyObject*args)
{
	char buffer[16];
	int s = socket(AF_INET,SOCK_DGRAM,0);
	const char*dns="8.8.8.8";
	
	struct sockaddr_in serv;
	memset(&serv,0,sizeof(serv));
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=inet_addr(dns);
	serv.sin_port=htons(53);
	
	int err = connect(s,(const struct sockaddr*)&serv,sizeof(serv));
	struct sockaddr_in n;
	socklen_t nl=sizeof(n);
	err=getsockname(s,(struct sockaddr*)&n,&nl);
	const char *p = inet_ntop(AF_INET,&n.sin_addr,buffer);
	
	close(s);
	return PyArg_BuildValue("s",buffer);
}

static PyObject * init_buf(PyObject*self,PyObject*args)
{
	char *sip,*dip;
	if(!PyArg_ParseTuple(args,"ss",sip,dip))
		return NULL;
	int nbytes;
	register long sum;
	register short answer;
	unsigned short *ptr,oddbyte;
	char buffer[256]; // should be enough
	struct iphdr *iph = (struct iphdr*)buffer;
	struct tcphdr *tcph=(struct tcphdr*)(buffer+sizeof(struct iphdr));
	iph->ihl	= 5;
	iph->version	= 4;
	iph->tos	= 0;
	iph->tot_len	= sizeof(struct tcphdr)+sizeof(struct iphdr);
	iph->id		= htons(54312);
	iph->frag_off	= htons(16384);
	iph->ttl	= 64;
	iph->protocol	= IPPROTO_TCP;
	iph->check	= 0; // later
	iph->saddr	= inet_addr(sip);
	iph->saddr	= inet_addr(dip);
	
	// checksum
	ptr = (unsigned short*)buffer;
	nbytes = iph->tot_len>>1;
	sum=0;
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
	sum=sum+(sum>>16);
	answer=(short)~sum;
	
	iph->check=(answer);
	
	tcph->source	= htons(34212); // source port
	tcph->dest	= htons(80);	// for now
	tcph->seq	= htonl(1105612138);
	tcph->ack_seq	= 0;
	tcph->doff	= sizeof(struct tcphdr)/4;
	tcph->fin	= 0;
	tcph->syn	= 0; // set later, flexibility
	tcph->rst	= 0;
	tcph->psh	= 0;
	tcph->ack	= 0;
	tcph->urg	= 0;
	tcph->window	= htons(14600);
	tcph->check	= 0; // kernel pls
	tcph->urg_ptr	= 0;
	
	return PyArg_BuildValue("s",buffer);
}
	
static PyObject * send_to(PyObject*self,PyObject*args)
{
	struct pseudo_header
	{
		u_int32_t	source_addr;
		u_int32_t	dest_addr;
		u_int8_t	placeholder;
		u_int8_t	protocol;
		u_int16_t	tcp_length;
		struct tcphdr tcp;
	}; // because fuck memory, right?
	struct sockaddr_in dest;
	memset(&dest,0,sizeof(dest));
	struct pseudo_header psh;
	int socket;
	char *ip,*buf;
	short port;
	if(!PyArg_ParseTuple(args,"ishs",&socket,&ip,port,buf)
		return NULL;
	if(inet_addr(ip)==-1)
	{
		PyErr_SetString(IPErr,"Ip is invalid");
		return NULL;
	}
	struct iphdr *iph=(struct iphdr*)buffer;
	struct tcphdr *tcph=(struct tcphdr*)(buffer+sizeof(struct iphdr));
	if(iph->daddr!=inet_addr(ip))
	{
		iph->check=0;
		iph->daddr=inet_addr(ip);
		// another checksum
		register long sum;
		register short answer;
		unsigned short *ptr,oddbyte;
		ptr = (unsigned short*)buffer;
		nbytes = iph->tot_len>>1;
		sum=0;
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
		sum=sum+(sum>>16);
		answer=(short)~sum;
	
		iph->check=(answer);
	}
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = iph->daddr;
	
	tcph->dest=htons(port);
	tcph->check=0;	// the kernel is supposed to solve the checksum of tcp header
	// but still ...
	psh.source_addr	= iph->saddr;
	psh.dest_addr	= iph->daddr;
	psh.placeholder	= 0;
	psh.protocol	= IPPROTO_TCP;
	psh.tcp_length	= htons(sizeof(struct tcphdr));
	memcpy(&psh.tcp,tcph,sizeof(struct tcphdr));
	
	// yet another checksum
	ptr=(unsigned short*)&psh;
	nbytes=sizeof(struct pseudo_header);
	sum=0;
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
	sum=sum+(sum>>16);
	answer=(short)~sum;
	tcph->check=answer;
	
	//send
	if(sendto(socket,buffer,sizeof(struct iphdr)+sizeof(struct tcphdr),0,(struct sockaddr *)&dest,sizeof(dest)) < 0)
	{
		PyErr_SetString(SendErr,"Could not send packet");
		return NULL;
	}
	return PyArg_BuildValue("i",0);
}

//method table
static PyMethodDef TscanMethods[] = {
	//...
	
	{NULL,NULL,0,NULL} //sentinel
}

//do on the go
PyMODINIT_FUNC
inittscan(void)
{
	PyObject *m;
	
	m=Py_InitModule("tscan",TscanMethods);
	if(m==NULL)
		return;
	
	// Exceptions
	IPErr = PyErr_NewException("ip.error",NULL,NULL);
	Py_INCREF(IPErr);
	PyModule_AddObject(m,"error",IPErr);
	SendErr = PyErr_NewException("send.error",NULL,NULL);
	Py_INCREF(SendErr);
	PyModule_AddObject(m,"error",SendErr);
}
