/**
 * @file moves_io.c
 * @brief Input/output functions for moves and board display
 * 
 * Provides functions for:
 * - Converting moves to algebraic notation (e2e4, e7e8q)
 * - Parsing algebraic notation to internal move representation
 * - Converting square indices to algebraic notation (e4, a1)
 * - Printing move lists
 * 
 * These functions are used by:
 * - UCI and XBoard protocol handlers
 * - Console interface
 * - Debugging and logging
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

// io.c

#include "stdio.h"
#include "types_definitions.h"

char *prSq(const int squareIndex) {

	static char SqStr[3];

	int file = g_filesBoard[squareIndex];
	int rank = g_ranksBoard[squareIndex];

	sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));

	return SqStr;

}

char *prMove(const int move) {

	static char MvStr[6];

	int ff = g_filesBoard[MOVE_GET_FROM_SQUARE(move)];
	int rf = g_ranksBoard[MOVE_GET_FROM_SQUARE(move)];
	int ft = g_filesBoard[MOVE_GET_TO_SQUARE(move)];
	int rt = g_ranksBoard[MOVE_GET_TO_SQUARE(move)];

	int promoted = MOVE_GET_PROMOTED(move);

	if(promoted) {
		char pchar = 'q';
		if(PIECE_IS_KNIGHT(promoted)) {
			pchar = 'n';
		} else if(PIECE_IS_ROOK_QUEEN(promoted) && !PIECE_IS_BISHOP_QUEEN(promoted))  {
			pchar = 'r';
		} else if(!PIECE_IS_ROOK_QUEEN(promoted) && PIECE_IS_BISHOP_QUEEN(promoted))  {
			pchar = 'b';
		}
		sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
	} else {
		sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
	}

	return MvStr;
}

int moveParse(const char *ptrChar, ChessBoard *board) {

	ASSERT(board->check());

	if(ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
    if(ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
    if(ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
    if(ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;

    int from = fILERANKTOSQUARE(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = fILERANKTOSQUARE(ptrChar[2] - 'a', ptrChar[3] - '1');

	ASSERT(sqOnBoard(from) && sqOnBoard(to));

	MoveList list[1];
    moveGenerateAll(board,list);
    int moveNum = 0;
	int move = 0;
	int promotedPiece = EMPTY;

	for(moveNum = 0; moveNum < list->size(); ++moveNum) {
		move = list->at(moveNum).raw();
		if(MOVE_GET_FROM_SQUARE(move)==from && MOVE_GET_TO_SQUARE(move)==to) {
			promotedPiece = MOVE_GET_PROMOTED(move);
			if(promotedPiece!=EMPTY) {
				if(PIECE_IS_ROOK_QUEEN(promotedPiece) && !PIECE_IS_BISHOP_QUEEN(promotedPiece) && ptrChar[4]=='r') {
					return move;
				} else if(!PIECE_IS_ROOK_QUEEN(promotedPiece) && PIECE_IS_BISHOP_QUEEN(promotedPiece) && ptrChar[4]=='b') {
					return move;
				} else if(PIECE_IS_ROOK_QUEEN(promotedPiece) && PIECE_IS_BISHOP_QUEEN(promotedPiece) && ptrChar[4]=='q') {
					return move;
				} else if(PIECE_IS_KNIGHT(promotedPiece)&& ptrChar[4]=='n') {
					return move;
				}
				continue;
			}
			return move;
		}
    }

    return NOMOVE;
}

void MoveList::print() const {
	printMoveList(this);
}

void printMoveList(const MoveList *list) {
	int index = 0;
	int score = 0;
	int move = 0;
	printf("MoveList:\n");

	for(index = 0; index < list->size(); ++index) {

		move = list->at(index).raw();
		score = list->at(index).score();

		printf("move:%d > %s (score:%d)\n",index+1,prMove(move),score);
	}
	printf("MoveList Total %d Moves:\n\n",list->size());
}














