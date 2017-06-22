/*
 * Turing Machine prototype
 * File I/O header file
 *
 * format:
 *	'<name>:<input><output><+/-/N><next>,<input><out(...)\n'
 *
 *
 * Copyright Tiago Teixeira, 2017
*/
#ifndef _TURING_FILE_IO_H_
#define _TURING_FILE_IO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct state
{
	int value;	// state name/designation
	char*ins;	// instructions
};

// read with inner malloc
struct state* read_states(const char*fname,int*len);
// free resources
void free_states(struct state* states,int len);
// get state by name
struct state get_state(struct state*states,int len, int value);

#endif
