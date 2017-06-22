/*
 * Turing Machine prototype
 * Main source file
 *
 * format:
 *	'<name>:<input><output><+/-/N><next>,<input><out(...)\n'
 *
 *
 * Copyright Tiago Teixeira, 2017
*/
#include "fileio.h"

char* do_state(struct state s,char*input,int*next)
{
	int act = 0;
	while(*(s.ins+act)!=0)
	{
		if(*(s.ins+act)==*input)
		{
			// cool
			*input=*(s.ins+act+1);
			sscanf(s.ins+act+3,"%d",next);
			if(*(s.ins+act+2)=='+')
				return ++input;
			else if(*(s.ins+act+2)=='-')
				return --input;
			else
				return NULL;
		}
		char*ph = strchr(s.ins+act,',');
		if(ph==NULL)
			return NULL;
		act+=(ph-(s.ins+act))+1;
	}
	return NULL;
}

void print_actual(char*input,char*act,int state_value)
{
	printf("(%d,",state_value);
	for(int i=0;*(input+i)!=0;i++)
	{
		if((input+i)==act)
			putchar('|');
		putchar(*(input+i));
		if((input+i)==act)
			putchar('|');
	}
	printf(")\n");
}

int main(int argc,char**argv)
{
	struct state* states = NULL,ns;
	int len;
	if(argc!=3)
	{
		fprintf(stderr,"Error: usage: %s <fname> <input_string>\n",*argv);
		return 1;
	}
	states=read_states(argv[1],&len);
	if(states==NULL)
		return 2;
	char *act = argv[2];
	int next = states[0].value;
	while(act!=NULL)
	{
		ns = get_state(states,len,next);
		print_actual(argv[2],act,ns.value);
		if(ns.ins==NULL)
		{
			fprintf(stderr,"Error: state: state non-existent\n");
			free_states(states,len);
			return 3;
		}
		act = do_state(ns,act,&next);
	}
	free_states(states,len);
	
	return 0;
}
