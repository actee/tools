#include <Python.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "help.h"
#include "structmember.h"

// exceptions
static PyObject *IPErr;
static PyObject *SendErr;
static PyObject *RecvErr;
static PyObject *SockErr;
static PyObject *ThreadErr;

//new object
typedef struct {
	PyObject_HEAD
	int socket;
	pthread_t sniffer;
	FILE* fd;
	unsigned int ip;
	char*buffer;
}	TSniffer;
static void TSniffer_dealloc(TSniffer*self)
{
	pthread_cancel(self->sniffer);
	close(self->socket);
	fclose(self->fd);
	free(self->buffer);
	Py_TYPE(self)->tp_free((PyObject*)self);
}
static PyObject*TSniffer_new(PyTypeObject *type,PyObject*args,PyObject*kwds)
{
	TSniffer *self;
	self=(TSniffer*)type->tp_alloc(type,0);
	if(self!=NULL)
		self->buffer=(char*)malloc(65536);
	return (PyObject*)self;
}
static int TSniffer_init(TSniffer*self,PyObject*args)
{
	const char*ip;
	if(!PyArg_ParseTuple(args,"s",&ip))
		return -1;
	if(inet_addr(ip)==-1)
	{
		PyErr_SetString(IPErr,"Invalid IP");
		return -1;
	}
	char fn[32];
	self->ip=inet_addr(ip);
	sprintf(fn,"/tmp/info_%s",ip);
	self->fd = fopen(fn,"w+b");
	if((self->socket=socket(AF_INET,SOCK_RAW,IPPROTO_TCP))<0)
	{
		PyErr_SetString(SockErr,"Could not create socket");
		return -1;
	}
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if(setsockopt(self->socket,SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		PyErr_SetString(SockErr,"Could not change soket options");
		return -1;
	}
	return 0;
}

static PyMemberDef TSniffer_members[] = {
	{"buffer",T_STRING,offsetof(TSniffer,buffer),0,"tsniffer buffer"},
	{"socket",T_INT,offsetof(TSniffer,socket),0,"tsniffer socket"},
//	{"ip",T_INT,offsetof(TSniffer,ip),0,"tsniffer ip"},
	{NULL},
};
/* Threading stuff */
int sniff(int socket,char*buffer,FILE*f,unsigned int ip)
{
	struct sockaddr saddr;
	memset(&saddr,0,sizeof saddr);
	socklen_t saddr_size = sizeof saddr;
	struct flags_tw info;
	struct iphdr *iph;
	struct tcphdr *tcph;
	//printf("Trying to receive\n");
	if(recvfrom(socket,buffer,65536,0,&saddr,&saddr_size) < 0)
	{
		return -1;
	}
	iph = (struct iphdr*)buffer;
	if(iph->protocol==6)
	{
		if(iph->saddr !=ip)
			return -1;
		tcph = (struct tcphdr*)(buffer+(iph->ihl*4));
		info.port = ntohs(tcph->source);
		info.flags = 0;
		info.flags = tcph->fin;
		info.flags = (info.flags << 1) + tcph->syn;
		info.flags = (info.flags << 1) + tcph->rst;
		info.flags = (info.flags << 1) + tcph->psh;
		info.flags = (info.flags << 1) + tcph->ack;
		info.flags = (info.flags << 1) + tcph->urg;
		if(fwrite(&info,sizeof info,1,f)!=1)
			return -2;
	}
	return 0;
}
void*recv_resp(void*ptr)
{
	/* cancelity */
	int old_state = PTHREAD_CANCEL_DISABLE;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
	struct sniffer_info si = *((struct sniffer_info*)ptr);
	free(ptr);
	while(1)
	{
		sniff(si.socket,si.buffer,si.fd,si.ip);
		pthread_testcancel();
	}
}
//methods
static PyObject*TSniffer_start(TSniffer*self)
{
	struct sniffer_info *si = malloc(sizeof(*si));
	si->socket = self->socket;
	si->ip = self->ip;
	si->fd = self->fd;
	si->buffer = self->buffer;
	if(pthread_create(&self->sniffer,NULL,recv_resp,si)<0)
	{
		PyErr_SetString(ThreadErr,"Could not create thread");
		return NULL;
	}
	return Py_BuildValue("i",0);
}
static PyObject*TSniffer_stop(TSniffer*self)
{
	while(!pthread_cancel(self->sniffer));
	fseek(self->fd,(long)0,SEEK_SET);
	return Py_BuildValue("i",0);
}
static PyObject*TSniffer_getinfo(TSniffer*self)
{
	struct flags_tw info;
	if(fread(&info,sizeof(struct flags_tw),1,self->fd)!= 1)
	{
		PyErr_SetString(PyExc_IOError,"Reached EOF");
		return NULL;
	}
	return Py_BuildValue("(hiiiiii)",info.port,(info.flags&32)/32,(info.flags&16)/16,(info.flags&8)/8,(info.flags&4)/4,(info.flags&2)/2,info.flags&1);
}

/* Get Set */
static int TSniffer_setip(TSniffer*self,char*ip,void*closure)
{
	if(inet_addr(ip)==self->ip)
	{
		return 0;
	}
	if(inet_addr(ip)==-1)
	{
		PyErr_SetString(IPErr,"Invalid IP");
		return -1;
	}
	char fn[32];
	while(!pthread_cancel(self->sniffer));
	fclose(self->fd);
	self->ip = inet_addr(ip);
	sprintf(fn,"/tmp/info_%s",ip);
	self->fd = fopen(fn,"w+b");
	return 0;
}
static PyObject*TSniffer_getip(TSniffer*self,void*closure)
{
	char ip[16];
	sprintf(ip,"%u.%u.%u.%u",(unsigned int)(self->ip%256),(unsigned int)((self->ip>>8)%256),(unsigned int)((self->ip>>16)%256),(unsigned int)((self->ip>>24)%256));
	return Py_BuildValue("s",ip);
}

static PyGetSetDef TSniffer_getseters[] = {
	{"ip",(getter)TSniffer_getip,(setter)TSniffer_setip,"ip",NULL},
	{NULL}
};
static PyMethodDef TSniffer_methods[] = {
	{"start",(PyCFunction)TSniffer_start,METH_NOARGS,"Start sniffing"},
	{"stop",(PyCFunction)TSniffer_stop,METH_NOARGS,"Stop sniffing"},
	{"get_info",(PyCFunction)TSniffer_getinfo,METH_NOARGS,"Get info"},
	{NULL}
};

static PyTypeObject TSnifferType = {
	PyObject_HEAD_INIT(NULL)
	0,	// size
	"tscanner.TSniffer", // name
	sizeof(TSniffer),	// basic size
	0,
	(destructor)TSniffer_dealloc,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
	"TSniffer",
	0,
	0,
	0,
	0,
	0,
	0,
	TSniffer_methods,
	TSniffer_members,
	TSniffer_getseters,
	0,
	0,
	0,
	0,
	0,
	(initproc)TSniffer_init,
	0,
	TSniffer_new,	// tp_new
};