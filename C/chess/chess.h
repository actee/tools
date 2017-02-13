#include <stdint.h>
#include <sys/socket.h>
#ifndef CHESS_HEADER
#define CHESS_HEADER
/*
 00000001	King	K
 00000010	Queen	Q
 00000011	Bishop	B
 00000100	Knight	N
 00000101	Tower	T
 00000110	Pawn	P
 10xxxxxx	Player 1	0x80
 01xxxxxx	Player 2	0x40
*/
/*
 Communication Protocol
 Packet
	u_char	p1.row
	u_char	p1.col
	u_char	p2.row
	u_char	p2.col
*/
#define PORT 8008
//board[row][col]
//colors
#define FNRM	"\x1B[0m"
#define	FGRN	"\x1B[32m"
#define FRED	"\x1B[31m"
#define BBLK	"\x1B[40m"
#define BWHT	"\x1B[47m"
#define KING	0x01
#define QUEEN	0x02
#define BISHOP	0x03
#define KNIGHT	0x04
#define TOWER	0x05
#define PAWN	0x06
#define PL1	0x80
#define PL2	0x40
#define NPL	0x07
struct position {
	char col;
	char row;
};
typedef unsigned char ichar;
// chess core
int	check_king(ichar**board,struct position king);
int	check_mate_king(ichar**board,struct position king);
int	move(ichar**board,struct position from,struct position to);
// chess ui
int print_board(ichar**board);
ichar piece_to_char(ichar piece);
ichar char_tp_piece(ichar name,int player);
int init_board(ichar**board);
//TODO
int connect_h(struct sockaddr_in);
int host_h(struct sockaddr_in);
int recv_info(int sock,struct position *p1, struct position *p2);
int send_info(int sock,struct position *p1, struct position *p2);
#endif
