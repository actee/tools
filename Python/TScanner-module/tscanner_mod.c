#include <Python.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#include "help.h"
#include "structmember.h"
#include "tsniffer_obj.h"
#include "methods.h"

// exceptions
static PyObject *IPErr;
static PyObject *SendErr;
static PyObject *RecvErr;
static PyObject *SockErr;
static PyObject *ThreadErr;

typedef struct {
	PyObject_HEAD
	struct pseudo_header psh;
	struct sockaddr_in dest;
	char *buffer;
	int socket;
}	TScanner;

static void TScanner_dealloc(TScanner*self)
{
	free(self->buffer);
	Py_TYPE(self)->tp_free((PyObject*)self);
}
static PyObject *TScanner_new(PyTypeObject*type, PyObject*args, PyObject*kwds)
{
	TScanner *self;
	self=(TScanner*)type->tp_alloc(type,0);
	if(self!=NULL)
	{
		self->buffer = (char*)malloc(128);
	}
	return (PyObject*)self;
}
static int TScanner_init(TScanner*self,PyObject*args)
{
	const char *ip;
	char sip[16];
	if(!PyArg_ParseTuple(args,"s",&ip))
		return -1;
	if(inet_addr(ip)==-1)
	{
		PyErr_SetString(IPErr,"Invalid IP");
		return -1;
	}
	//self->buffer = (char*)malloc(128); // should be enough
	memset(self->buffer,0,128);
	// get local address
	self->socket=socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in serv;
	memset(&serv,0,sizeof(serv));
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=inet_addr("8.8.8.8");
	serv.sin_port=htons(53);
	if(connect(self->socket,(const struct sockaddr*)&serv,sizeof(serv))==-1)
	{
		PyErr_SetString(SockErr,"Could not get local ip");
		return -1;
	}
	struct sockaddr_in name;
	socklen_t namel=sizeof(name);
	getsockname(self->socket,(struct sockaddr*)&name,&namel);
	inet_ntop(AF_INET,&name.sin_addr,sip,sizeof(name));
	close(self->socket);
	//reuse socket
	if((self->socket=socket(PF_INET,SOCK_RAW,IPPROTO_TCP))==-1)
	{
		PyErr_SetString(SockErr,"Could not initialize socket");
		return -1;
	}
	// raw
	int one=1;const int *val=&one;
	if(setsockopt(self->socket,IPPROTO_IP,IP_HDRINCL,val,sizeof(one))<0)
	{
		PyErr_SetString(SockErr,"Could not turn to raw");
		return -1;
	}
	struct iphdr *iph=(struct iphdr*)self->buffer;
	struct tcphdr *tcph=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	// init stuff
	iph->ihl	= 5;
	iph->version	= 4;
	iph->tos	= 0;
	iph->tot_len	= sizeof(struct iphdr)+sizeof(struct tcphdr);
	iph->id		= htons(54312);
	iph->frag_off	= htons(16384);
	iph->ttl	= 64;
	iph->protocol	= IPPROTO_TCP;
	iph->check	= 0; // later
	iph->saddr	= inet_addr(sip);
	iph->daddr	= inet_addr(ip);
	iph->check	= csum((unsigned short*)self->buffer,iph->tot_len>>1);
	//tcp
	tcph->source	= htons(34138);
	tcph->dest	= htons(1);
	tcph->seq	= htonl(1105612138);
	tcph->ack_seq	= 0;
	tcph->doff	= sizeof(struct tcphdr)/4;
	tcph->fin	= 0;
	tcph->syn	= 0;
	tcph->rst	= 0;
	tcph->psh	= 0;
	tcph->ack	= 0;
	tcph->urg	= 0;
	tcph->window	= htons(14600);
	tcph->check	= 0; // kernel pls
	tcph->urg_ptr	= 0;
	
	self->dest.sin_family=AF_INET;
	self->dest.sin_addr.s_addr=iph->daddr;
	self->psh.source_addr=iph->saddr;
	self->psh.dest_addr=iph->daddr;
	self->psh.placeholder=0;
	self->psh.protocol=IPPROTO_TCP;
	self->psh.tcp_length=htons(sizeof(struct tcphdr));
	// no need conversion right?
	return 0;
}

static PyMemberDef TScanner_members[] = {
	// try not declaring psh and dest
	{"buffer",T_STRING,offsetof(TScanner,buffer),0,"tscanner buffer"},
	{"socket",T_INT,offsetof(TScanner,socket),0,"tscanner socket"},
	{NULL} // sentinel
};

/* methods now */
// set ip
static PyObject* TScanner_setip(TScanner*self,PyObject*args)
{
	const char *new_ip;
	if(!PyArg_ParseTuple(args,"s",&new_ip))
		return NULL;
	if(inet_addr(new_ip)==-1)
	{
		PyErr_SetString(IPErr,"Invalid IP");
		return NULL;
	}
	struct iphdr *iph=(struct iphdr*)self->buffer;
	iph->daddr = inet_addr(new_ip);
	iph->check=0;
	iph->check=csum((unsigned short*)self->buffer,iph->tot_len>>1);
	self->psh.dest_addr=iph->daddr;
	return Py_BuildValue("i",0);
}
// set flags
// testing different stuff
static int TScanner_setsyn(TScanner*self,int f,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->syn	= f<=0?0:1;
	return 0;
}
static PyObject*TScanner_getsyn(TScanner*self,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	int f=t->syn==1?1:0;
	return Py_BuildValue("i",f);
}
static int TScanner_setfin(TScanner*self,int f,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->fin	= f<=0?0:1;
	return 0;
}
static PyObject*TScanner_getfin(TScanner*self,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	int f=t->fin==1?1:0;
	return Py_BuildValue("i",f);
}
static int TScanner_setrst(TScanner*self,int f,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->rst	= f<=0?0:1;
	return 0;
}
static PyObject*TScanner_getrst(TScanner*self,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	int f=t->rst==1?1:0;
	return Py_BuildValue("i",f);
}
static int TScanner_setpsh(TScanner*self,int f,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->psh	= f<=0?0:1;
	return 0;
}
static PyObject*TScanner_getpsh(TScanner*self,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	int f=t->psh==1?1:0;
	return Py_BuildValue("i",f);
}
static int TScanner_setack(TScanner*self,int f,void*closure)
{
	struct tcphdr*t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->ack=f<=0?0:1;
	return 0;
}
static PyObject*TScanner_getack(TScanner*self,void*closure)
{
	struct tcphdr *t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	int f=t->ack==1?1:0;
	return Py_BuildValue("i",f);
}
//urg should not be needed
static PyObject*TScanner_rstflags(TScanner*self)
{
	struct tcphdr*t=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	t->fin=0;
	t->syn=0;
	t->rst=0;
	t->psh=0;
	t->ack=0;
	return Py_BuildValue("i",0);
}
// send packet
static PyObject*TScanner_send(TScanner*self,PyObject*args)
{
	short p;
	if(!PyArg_ParseTuple(args,"h",&p))
		return NULL;
	struct tcphdr*tcph=(struct tcphdr*)(self->buffer+sizeof(struct iphdr));
	tcph->dest=htons(p);
	tcph->check=0;
	memcpy(&self->psh.tcp,tcph,sizeof(struct tcphdr));
	tcph->check=csum((unsigned short*)&self->psh,sizeof(struct pseudo_header));
	//send
	if(sendto(self->socket,self->buffer,sizeof(struct iphdr)+sizeof(struct tcphdr),0, (struct sockaddr*)&self->dest,sizeof(self->dest))<0)
	{
		PyErr_SetString(SendErr,"Could not send");
		return NULL;
	}
	return Py_BuildValue("i",0);
}
static PyGetSetDef TScanner_getseters[] = {
	{"fin",(getter)TScanner_getfin,(setter)TScanner_setfin, "fin flag",NULL},
	{"syn",(getter)TScanner_getsyn,(setter)TScanner_setsyn, "syn flag",NULL},
	{"rst",(getter)TScanner_getrst,(setter)TScanner_setrst, "rst flag",NULL},
	{"psh",(getter)TScanner_getpsh,(setter)TScanner_setpsh, "psh flag",NULL},
	{"ack",(getter)TScanner_getack,(setter)TScanner_setack, "ack flag",NULL},
	{NULL} // sentinel
};
static PyMethodDef TScanner_methods[] = {
	{"setIP",(PyCFunction)TScanner_setip,METH_VARARGS,"Set the target IP"},
	{"send",(PyCFunction)TScanner_send,METH_VARARGS,"Send the packet to the specified port"},
	{"rstflags",(PyCFunction)TScanner_rstflags,METH_NOARGS,"Reset the flags"},
	{NULL}
};

static PyTypeObject TScannerType = {
	PyObject_HEAD_INIT(NULL)
	0,				/* ob_size */
	"tscanner.TScanner",		/* tp_name */
	sizeof(TScanner),		/* tp_basicsize */
	0,				/* tp_itemsize */
	(destructor)TScanner_dealloc,	/* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	"TScanner",
	0,
	0,
	0,
	0,
	0,
	0,
	TScanner_methods,
	TScanner_members,
	TScanner_getseters,
	0,
	0,
	0,
	0,
	0,
	(initproc)TScanner_init,
	0,
	TScanner_new,			/* tp_new */
};

static PyMethodDef module_methods[] = {
	{"dns",(PyCFunction)TScanner_dns,METH_VARARGS,"Get IP from hostname"},
	{NULL} // sentinel
};

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inittscanner(void)
{
	PyObject *mod;
	if(PyType_Ready(&TScannerType) < 0)
		return;
	if(PyType_Ready(&TSnifferType) < 0)
		return;
	mod = Py_InitModule3("tscanner", module_methods,"Scanner Module with TScanner.");
	if(mod==NULL)
		return;
	
	//Types
	Py_INCREF(&TScannerType);
	PyModule_AddObject(mod,"TScanner",(PyObject*)&TScannerType);
	Py_INCREF(&TSnifferType);
	PyModule_AddObject(mod,"TSniffer",(PyObject*)&TSnifferType);
	
	//Exceptions
	IPErr = PyErr_NewException("ip.error",NULL,NULL);
	Py_INCREF(IPErr);
	PyModule_AddObject(mod,"error",IPErr);
	SendErr = PyErr_NewException("send.error",NULL,NULL);
	Py_INCREF(SendErr);
	PyModule_AddObject(mod,"error",SendErr);
	RecvErr = PyErr_NewException("recv.error",NULL,NULL);
	Py_INCREF(RecvErr);
	PyModule_AddObject(mod,"error",RecvErr);
	SockErr = PyErr_NewException("socket.error",NULL,NULL);
	Py_INCREF(SockErr);
	PyModule_AddObject(mod,"error",SockErr);
	ThreadErr = PyErr_NewException("thread.error",NULL,NULL);
	Py_INCREF(ThreadErr);
	PyModule_AddObject(mod,"error",ThreadErr);
}
