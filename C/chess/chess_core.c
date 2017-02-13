/*
 Core file for chess game
 
 copyright 2017 Tiago Teixeira
*/
#include "chess.h"
int check_mate_king(ichar**board,struct position king)
{
	int i,j;
	struct position cm;
	cm.row=king.row;
	cm.col=king.col;
	if(!check_king(board,king))
		return 0;
	cm.row++;// down
	if(cm.row<8)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.col++; // down,right
	if(cm.row<8&&cm.col<8)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.row--;//right
	if(cm.col<8)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.row--;//up right
	if(cm.col<8&&cm.row>=0)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.col--;//up
	if(cm.row>=0)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.col--;// up left
	if(cm.row>=0&&cm.col>=0)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.row++;//left
	if(cm.col>=0)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	cm.row++;//down left
	if(cm.row<8&&cm.col>=8)
		if(board[cm.row][cm.col]==' '&&!check_king(board,cm))
			return 0;
	// is checked on every possible position
	return 1;
}
int check_king(ichar**board,struct position king)
{
	int i,j;
	char pl;
	pl=board[king.row][king.col]&0xc0;
	pl^=0xc0;	// opp player
	//knight
	i=king.row;
	j=king.col;
	if(i+2<8&&j+1<8)
		if(board[i+2][j+1]==(pl|KNIGHT))
			return 1;
	if(i+1<8&&j+2<8)
		if(board[i+1][j+2]==(pl|KNIGHT))
			return 1;
	if(i+2<8&&j-1>=0)
		if(board[i+2][j-1]==(pl|KNIGHT))
			return 1;
	if(i+1<8&&j-2>=0)
		if(board[i+1][j-2]==(pl|KNIGHT))
			return 1;
	if(i-2>=0&&j-1>=0)
		if(board[i-2][j-1]==(pl|KNIGHT))
			return 1;
	if(i-1>=0&&j-2>=0)
		if(board[i-1][j-2]==(pl|KNIGHT))
			return 1;
	if(i-2>=0&&j+1<8)
		if(board[i-2][j+1]==(pl|KNIGHT))
			return 1;
	if(i-1>=0&&j+2<8)
		if(board[i-1][j+2]==(pl|KNIGHT))
			return 1;
	//pawn
	if(board[king.row][king.col]&PL1)
	{
		if(i+1<8&&j+1<8)
			if(board[i+1][j+1]==(PAWN|PL2))
				return 1;
		if(i+1<8&&j-1>=0)
			if(board[i+1][j-1]==(PAWN|PL2))
				return 1;
	}
	if(board[king.row][king.col]&PL2)
	{
		if(i-1>=0&&j+1<8)
			if(board[i+1][j+1]==(PAWN|PL1))
				return 1;
		if(i-1>=0&&j-1>=0)
			if(board[i+1][j-1]==(PAWN|PL1))
				return 1;
	}
	// diagonal
	for(i=king.row,j=king.col;i<8&&j<8;i++,j++)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|BISHOP))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;i>=0&&j<8;i--,j++)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|BISHOP))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;i>=0&&j>=0;i--,j--)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|BISHOP))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;i<8&&j>=0;i++,j--)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|BISHOP))
			return 1;
		else
			break;
	}
	// straight
	for(i=king.row,j=king.col;i<8;i++)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|TOWER))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;i>=0;i--)
	{
		if(board[i][j]==' '||i==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|TOWER))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;j<8;j++)
	{
		if(board[i][j]==' '||j==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|TOWER))
			return 1;
		else
			break;
	}
	for(i=king.row,j=king.col;j>=0;j--)
	{
		if(board[i][j]==' '||j==king.row)
			continue;
		if(board[i][j]==(pl|QUEEN)||board[i][j]==(pl|TOWER))
			return 1;
		else
			break;
	}
	// if everything else fails
	return 0;
}
int move(ichar**board,struct position f,struct position t)
{
	int i,j,a,b,x,y;
	ichar pl,opl,h;
	pl=board[f.row][f.col]&0xc0;
	opl=pl^0xc0;
	i=t.row-f.row;
	j=t.col-f.col;
	a=i<0?i*(-1):i;
	b=j<0?j*(-1):j;
	// DEBUG printf("a %d b %d i %d j %d pl %hhx opl %hhx PL1 %hhx PL2 %hhx\n",a,b,i,j,pl,opl,PL1,PL2);
	if((board[t.row][t.col]&0xc0)==pl)
		return -1;
	if((board[f.row][f.col]&NPL)==KING)
	{
		// alright
		if(a>1||b>1)
			return -1;
		h=board[t.row][t.col];
		board[t.row][t.col]=board[f.row][f.col];
		board[f.row][f.col]=' ';
		if(check_king(board,t))
		{
			board[f.row][f.col]=board[t.row][t.col];
			board[t.row][t.col]=h;
			return -1;
		}
		else
			return 0;
	}
	if((board[f.row][f.col]&NPL)==KNIGHT)
	{
		if((a==1&&b==2)||(a==2&&b==1))
		{
			// legal knight move
			if((board[t.row][t.col]&0xc0)==pl)
				return -1;
			else
			{
				// move
				board[t.row][t.col]=board[f.row][f.col];
				board[f.row][f.col]=' ';
				return 0;
			}
		}
		else
			return -1;
	}
	if((board[f.row][f.col]&NPL)==PAWN)
	{
		// okay
		//printf("Its a pawn\n");
		if(pl==PL1&&i<0)
			return -1;
		if(pl==PL2&&i>0)
			return -1;
		if(!b&&board[t.row][t.col]==' ')
		{
			// simple forward move
			if(a==2)
			{
				// ho-ho
				if(pl==PL1&&f.row==1)
				{
					board[t.row][t.col]=board[f.row][f.col];
					board[f.row][f.col]=' ';
					return 0;
				}
				else if(pl==PL2&&f.row==6)
				{
					board[t.row][t.col]=board[f.row][f.col];
					board[f.row][f.col]=' ';
					return 0;
				}
				else
					return -1;
			}
			else if(a==1)
			{
				board[t.row][t.col]=board[f.row][f.col];
				board[f.row][f.col]=' ';
				return 0;
			}
			else
				return -1;
		}
		else if(b==1&&a==1&&((board[t.row][t.col]&0xc0)==opl))
		{
			// destroy
			board[t.row][t.col]=board[f.row][f.col];
			board[f.row][f.col]=' ';
			return 0;
		}
		// else illegal move
		else
			return -1;
	}
	if(a==b)
	{
		// diagonal, queen or bishop
		if(((board[f.row][f.col]&NPL)!=QUEEN)&&((board[f.row][f.col]&NPL)!=BISHOP))
			return -1;
		for(x=1,y=1;x<(a-1)&&y<(b-1);x++,y++)
			if(board[f.row+(x*(a/i))][f.col+(y*(b/j))]!=' ')
				return -1;
		// clear, move
		board[t.row][t.col]=board[f.row][f.col];
		board[f.row][f.col]=' ';
		return 0;
	}
	if(!a)
	{
		if(((board[f.row][f.col]&NPL)!=TOWER)&&((board[f.row][f.col]&NPL)!=QUEEN))
			return -1;
		// side move
		for(y=1;y<(b-1);y++)
			if(board[f.row][f.col+(y*(b/j))]!=' ')
				return -1;
		board[t.row][t.col]=board[f.row][f.col];
		board[f.row][f.col]=' ';
		return 0;
	}
	if(!b)
	{
		if(((board[f.row][f.col]&NPL)!=TOWER)&&((board[f.row][f.col]&NPL)!=QUEEN))
			return -1;
		// up/down move
		for(x=1;x<(a-1);x++)
			if(board[f.row+(x*(a/i))][f.col]!=' ')
				return -1;
		board[t.row][t.col]=board[f.row][f.col];
		board[f.row][f.col]=' ';
		return 0;
	}
	return 0;
}
