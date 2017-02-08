#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

static PyObject*TScanner_dns(PyObject*self,PyObject*args)
{
	const char *host;
	if(!PyArg_ParseTuple(args,"s",&host))
		return NULL;
	char ip[16];
	struct addrinfo hints;
	struct addrinfo *r;
	struct sockaddr_in *res;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	int sig = getaddrinfo(host,NULL,&hints,&r);
	if(sig!=0)
	{
		PyErr_SetString(IPErr,"Could not resolve hostname");
		return NULL;
	}
	res = (struct sockaddr_in*)r->ai_addr;
	unsigned long addr = res->sin_addr.s_addr;
	sprintf(ip,"%lu.%lu.%lu.%lu",addr%256,(addr>>8)%256,(addr>>16)%256,(addr>>24)%256);
	PyObject *ret = Py_BuildValue("s",ip);
	freeaddrinfo(r);
	return ret;
}