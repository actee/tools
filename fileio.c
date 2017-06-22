/*
 * Turing Machine prototype
 * File I/O source file
 *
 * format:
 *	'<name>:<input><output><+/-/N><next>,<input><out(...)\n'
 *
 *
 * Copyright Tiago Teixeira, 2017
*/
#include "fileio.h"

struct state* read_states(const char*fname,int *len)
{
	char buffer[256];
	struct state*states=NULL,*ph,s;
	FILE*statef;
	int l;
	*len=0;
	statef = fopen(fname,"r");
	if(statef==NULL)
	{
		fprintf(stderr,"Error: fopen: failed to open file\n");
		return NULL;
	}
	while(fscanf(statef,"%d:%s",&s.value,buffer)==2)
	{
		s.ins = (char*)malloc(strlen(buffer)+1);
		if(s.ins==NULL)
		{
			fprintf(stderr,"Error: malloc: failed to allocate memory\n");
			if(*len)
				free_states(states,*len);
			return NULL;
		}
		l=strlen(buffer);
		memcpy(s.ins,buffer,l);
		s.ins[l]=0;	// sentinel
		(*len)++;
		ph=states;
		states=(struct state*)malloc((*len)*sizeof s);
		if(states==NULL)
		{
			fprintf(stderr,"Error: malloc: failed to allocate memory\n");
			if(((*len)-1))
				free_states(states,(*len)-1);
			return NULL;
		}
		memcpy(states,ph,((*len)-1)*sizeof s);
		*(states+(*len)-1)=s;
		free(ph);
	}
	return states;
}

void free_states(struct state*states,int len)
{
	for(int i=0;i<len;i++)
		free((states+i)->ins);
	free(states);
}

struct state get_state(struct state*states,int len,int value)
{
//	printf("Looking for %d\n",value);
	for(int i=0;i<len;i++)
		if((states+i)->value==value)
			return *(states+i);
	struct state error;
	error.value = -1;
	error.ins = NULL;
	return error;
}
