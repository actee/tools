/*
 User interface for Chess Game
 Don't be fooled,
 still terminal/text only
 
 copyright 2017 Tiago Teixeira
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "chess.h"
int main(int argc,char**argv)
{
	if(argc<3)
	{
		printf("usage: %s [h|c] <ip> [port]\n",argv[0]);
		return 1;
	}
	if(inet_addr(argv[2])==-1)
	{
		printf("invalid IP\n");
		return 1;
	}
	if(*argv[1]!='h'&&*argv[1]!='c')
	{
		printf("invalid argument \'%s\'\n",argv[1]);
		return 1;
	}
	struct sockaddr_in server;
	int s;
	ichar player;
	struct position p1,p2;
	struct position k1,k2;
	int i/*,j*/;
	int cont;
	char h;
	ichar ptr[64];
	memset(ptr,0x20,64);
	ichar *board[8];
	// connections
	server.sin_family=AF_INET;
	server.sin_port=argc==4&&atoi(argv[3])?(short)atoi(argv[3]):PORT;
	server.sin_addr.s_addr=inet_addr(argv[2]);
	player=*argv[1]=='h'?PL1:PL2;
	if(player==PL1)
		s=host_h(server);
	else
		s=connect_h(server);
	//init board
	for(i=0;i<8;i++)
		board[i]=(ptr+(8*i));
	for(i=0;i<8;i++)
		memset(board[i],0x20,8);
	init_board(board);
	k1.row=0;
	k1.col=4;
	k2.row=7;
	k2.col=4;
	print_board(board);
	// main loop
	cont=1;
	if(player==PL2)
	{
		if(recv_info(s,&p1,&p2)<0)
		{
			close(s);
			printf("error receiving info\n");
			return 1;
		}
		move(board,p1,p2);
		print_board(board);
	}
	while(cont)
	{
		// play
		do
		{
			scanf(" %c%c%c%c",&p1.row,&p1.col,&p2.row,&p2.col);
			p1.row-=97;
			p1.col-=49;
			p2.row-=97;
			p2.col-=49;
			if(p1.row<0||p1.row>7||p1.col<0||p1.col>7||p2.row<0||p2.row>7||p2.col<0||p2.col>7)
			{
				printf("invalid coordinates\n");
				continue;
			}
			if((board[p1.row][p2.col]&0xc0)!=player)
			{
				printf("not your piece\n");
				continue;
			}
			if(k1.row==p1.row&&k1.col==p1.col)
			if((h=move(board,p1,p2))==-1)
			{
				printf("invalid move\n");
				continue;
			}
		}while(h==-1);
		if(board[p2.row][p2.col]==(KING|PL1))	// king moved
		{
			k1.row=p2.row;
			k1.col=p2.col;
		}
		if(board[p2.row][p2.col]==(KING|PL2))
		{
			k2.row=p2.row;
			k2.col=p2.col;
		}
		// check?
		if(check_king(board,k1))
		{
			if(check_mate_king(board,k1))
			{
				printf("King 1 on CHECK MATE\n");
				printf("Player 2 Wins!\n");
				close(s);
				return 0;
			}
			else
				printf("King 1 on CHECK\n");
		}
		if(check_king(board,k2))
		{
			if(check_mate_king(board,k2))
			{
				printf("King 2 on CHECK MATE\n");
				printf("Player 1 Wins!\n");
				close(s);
				return 0;
			}
			else
				printf("King 2 on CHECK\n");
		}
		if(send_info(s,&p1,&p2)<0)
		{
			printf("error sending info\n");
			close(s);
			return 1;
		}
		print_board(board);
		if(recv_info(s,&p1,&p2)<0)
		{
			printf("error receiving info\n");
			close(s);
			return 1;
		}
		move(board,p1,p2);
		if(board[p2.row][p2.col]==(KING|PL1))	// king moved
		{
			k1.row=p2.row;
			k1.col=p2.col;
		}
		if(board[p2.row][p2.col]==(KING|PL2))
		{
			k2.row=p2.row;
			k2.col=p2.col;
		}
		// check ?
		if(check_king(board,k1))
		{
			if(check_mate_king(board,k1))
			{
				printf("King 1 on CHECK MATE\n");
				printf("Player 2 Wins!\n");
				close(s);
				return 0;
			}
			else
				printf("King 1 on CHECK\n");
		}
		if(check_king(board,k2))
		{
			if(check_mate_king(board,k2))
			{
				printf("King 2 on CHECK MATE\n");
				printf("Player 1 Wins!\n");
				close(s);
				return 0;
			}
			else
				printf("King 2 on CHECK\n");
		}
		print_board(board);
	}
	return 0;
}
ichar piece_to_char(ichar piece)
{
	ichar ret;
	piece&=NPL;
	ret=piece==1?'K':piece==2?'Q':piece==3?'B':piece==4?'N':piece==5?'T':piece==6?'p':' ';
	return ret;
}
ichar char_to_piece(ichar n,int p)
{
	ichar ret;
	n-=n>0x60?0x20:0;
	ret=n=='K'?1:n=='Q'?2:n=='B'?3:n=='N'?4:n=='T'?5:6;
	ret|=p==1?0x80:0x40;
	return ret;
}
int print_board(ichar**board)
{
	int i,j;
	system("clear");
	printf(FNRM);
	printf("    1  2  3  4  5  6  7  8\n");
	for(i=0;i<8;i++)
	{
		printf(FNRM);
		putchar(' ');
		putchar((char)(i+0x61));
		putchar(' ');
		for(j=0;j<8;j++)
		{
			if((i+j)%2)
				printf(BWHT);
			else
				printf(BBLK);
			if(board[i][j]&0x80)
				printf(FGRN);
			else 
				printf(FRED);
			putchar(' ');
			putchar(piece_to_char(board[i][j]));
			putchar(' ');
		}
		printf(FNRM);
		putchar('\n');
	}
	return 0;
}
int init_board(ichar**board)
{
	int i;
	board[0][0]=char_to_piece('T',1);
	board[0][1]=char_to_piece('N',1);
	board[0][2]=char_to_piece('B',1);
	board[0][3]=char_to_piece('Q',1);
	board[0][4]=char_to_piece('K',1);
	board[0][5]=char_to_piece('B',1);
	board[0][6]=char_to_piece('N',1);
	board[0][7]=char_to_piece('T',1);
	board[7][0]=char_to_piece('T',2);
	board[7][1]=char_to_piece('N',2);
	board[7][2]=char_to_piece('B',2);
	board[7][3]=char_to_piece('Q',2);
	board[7][4]=char_to_piece('K',2);
	board[7][5]=char_to_piece('B',2);
	board[7][6]=char_to_piece('N',2);
	board[7][7]=char_to_piece('T',2);
	for(i=0;i<8;i++)
	{
		board[1][i]=char_to_piece('P',1);
		board[6][i]=char_to_piece('P',2);
	}
	return 0;
}
int connect_h(struct sockaddr_in server)
{
	int ret;
	ret=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(ret==-1)
		return ret;
	if(connect(ret,(struct sockaddr*)&server,sizeof server)<0)
	{
		close(ret);
		return -1;
	}
	return ret;
}
int host_h(struct sockaddr_in server)
{
	int serv,ret,ip;
	struct sockaddr_in remote;
	int rem_size = sizeof remote;
	serv=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(serv==-1)
		return -1;
	if(bind(serv,(struct sockaddr*)&server,sizeof server)<0)
	{
		printf("error binding socket listener\n");
		close(serv);
		return -1;
	}
	listen(serv,1);
	ret=accept(serv,(struct sockaddr*)&remote,&rem_size);
	ip=remote.sin_addr.s_addr;
	printf("accepted from %hhu.%hhu.%hhu.%hhu\n",(char)(ip%256),(char)((ip>>8)%256),(char)((ip>>16)%256),(char)((ip>>24)%256));
	close(serv);
	return ret;
}
int send_info(int sock,struct position *p1,struct position *p2)
{
	ichar bytes[4];
	bytes[0]=p1->row;
	bytes[1]=p1->col;
	bytes[2]=p2->row;
	bytes[3]=p2->col;
	return send(sock,bytes,4,0);
}
int recv_info(int sock,struct position *p1,struct position *p2)
{
	ichar bytes[4];
	int ret = recv(sock,bytes,4,0);
	if(ret<0)
		return ret;
	p1->row=bytes[0];
	p1->col=bytes[1];
	p2->row=bytes[2];
	p2->col=bytes[3];
	return 0;
}
