/*
 hgame_server.c functions declarations
 
 Copyright 2017 Tiago Teixeira
 This file is part of hgameOS
*/

#define SERVER	"127.0.0.1"	// loopback debugging, change later
#define	PORT	8000
#define LOGINF	"logins.bin"
#define MAXCON	8		// max connections, server on Wii

#define S_AUTH	0
#define	S_CMD	1
#define S_RCN	2
#define S_CRK	3
#define S_SRV	4
#define S_DUL	5
#define S_CSW	6
#define S_CFE	10

/* Commands */
#define LS	6
#define START	10
#define KILL	11
#define RM	14
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

#define BSIZE	1024

int auth(unsigned char * buffer);	// returns number of bytes to send
int cmd_exec(unsigned char * buffer);
int list(unsigned char * buffer);
int check_file_exists(unsigned char *buffer);
int create_software(unsigned char *buffer);
int nmap(unsigned char*buffer);
int start_p(unsigned char*buffer);
int kill_p(unsigned char*buffer);
int crack(unsigned char *buffer);
int down_up_load(unsigned char*buffer);
void check_error(int err);
int remove_file(unsigned char*buffer);
int fuzz(unsigned char*buffer);
int crack_otg(unsigned char*buffer);
unsigned char code_to_char(unsigned char c);
