/*
 Functions and structs declarations
 
 Copyright 2017 Tiago Teixeira
 This file is part of hgameOS
*/
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* Server stuff */
#define SERVER	"127.0.0.1"	// loopback debugging, change later
#define S_PORT	8000
/* op codes */
#define	S_AUTH	0
#define	S_CMD	1
#define	S_RCN	2
#define S_CRK	3
#define S_SRV	4
#define S_DUL	5
#define	S_CSW	6
#define S_CFE	10

/* IP stuff */
#define NULL_IP	-1
#define LCLHOST	0

/* Command OP Codes */
#define CONERR	-1
#define ERROR	0
#define	CONNECT	1
#define LOGOUT	2
#define POWOFF	3
#define HELP	4
#define CLEAR	5
#define LS	6
#define NMAP	7
#define CREAT	8
#define ACTIVE	9
#define START	10
#define KILL	11
#define UPLD	12
#define DNLD	13
#define RM	14
#define EXEC	16
#define FUZZ	17
#define CRACK	18

/* Software */
#define	SERVICE	12
#define CRACKER	13

#define	DOS	31
#define	INJ	32
#define BOF	33
#define SPOOF	34
#define MITM	35
#define EOP	36

#define SSH	22
#define FTP	21

/* structs */
struct cmd {
	int cmd_opt1;
	int cmd_opt2;
	int cmd_opt3;
};

/* hgame.c */
void prompt(const char* uname,int ip);
int auth(int*sock,unsigned char *buffer);
/* parser.c */
int parse_cmd(struct cmd* opts,char*helper);
void parse_nmap(unsigned char*buffer);
void clean_stdin(void);
/* exec.c */
int exec(int * sock, int own_ip, unsigned char *buffer, const char* fname);
unsigned char char_to_op(char c);
/* services.c */
int create_lobby(unsigned char *buffer);
int create_service(unsigned char * buffer);
int create_cracker(unsigned char * buffer);
int read_int(int min,int max);
