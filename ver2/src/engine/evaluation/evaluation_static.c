/**
 * @file evaluation_static.c
 * @brief Position evaluation function
 * 
 * Evaluates chess positions by considering:
 * - Material balance (piece values)
 * - Piece-square tables (positional bonuses)
 * - Pawn structure (passed pawns, isolated pawns, doubled pawns)
 * - Piece mobility and activity
 * - King safety
 * - Bishop pair bonus
 * 
 * Returns a score in centipawns from the perspective of the side to move.
 * Positive scores favor the current player, negative scores favor the opponent.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

// evaluate.c

#include "stdio.h"
#include "types_definitions.h"

const int PawnIsolated = -10;
const int PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 }; 
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

const int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int KnightTable[64] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0		
};

const int BishopTable[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int RookTable[64] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0		
};

const int KingE[64] = {	
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	
};

const int KingO[64] = {	
	0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70		
};
// sjeng 11.2
//8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41 
int MaterialDraw(const ChessBoard *board) {

	ASSERT(Board_Check(board));
	
    if (!board->pieceCount[PIECE_TYPE_WHITE_ROOK] && !board->pieceCount[PIECE_TYPE_BLACK_ROOK] && !board->pieceCount[PIECE_TYPE_WHITE_QUEEN] && !board->pieceCount[PIECE_TYPE_BLACK_QUEEN]) {
	  if (!board->pieceCount[PIECE_TYPE_BLACK_BISHOP] && !board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) {
	      if (board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] < 3 && board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] < 3) {  return BOOL_TYPE_TRUE; }
	  } else if (!board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] && !board->pieceCount[PIECE_TYPE_BLACK_KNIGHT]) {
	     if (abs(board->pieceCount[PIECE_TYPE_WHITE_BISHOP] - board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) < 2) { return BOOL_TYPE_TRUE; }
	  } else if ((board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] < 3 && !board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) || (board->pieceCount[PIECE_TYPE_WHITE_BISHOP] == 1 && !board->pieceCount[PIECE_TYPE_WHITE_KNIGHT])) {
	    if ((board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] < 3 && !board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) || (board->pieceCount[PIECE_TYPE_BLACK_BISHOP] == 1 && !board->pieceCount[PIECE_TYPE_BLACK_KNIGHT]))  { return BOOL_TYPE_TRUE; }
	  }
	} else if (!board->pieceCount[PIECE_TYPE_WHITE_QUEEN] && !board->pieceCount[PIECE_TYPE_BLACK_QUEEN]) {
        if (board->pieceCount[PIECE_TYPE_WHITE_ROOK] == 1 && board->pieceCount[PIECE_TYPE_BLACK_ROOK] == 1) {
            if ((board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] + board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) < 2 && (board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] + board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) < 2)	{ return BOOL_TYPE_TRUE; }
        } else if (board->pieceCount[PIECE_TYPE_WHITE_ROOK] == 1 && !board->pieceCount[PIECE_TYPE_BLACK_ROOK]) {
            if ((board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] + board->pieceCount[PIECE_TYPE_WHITE_BISHOP] == 0) && (((board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] + board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) == 1) || ((board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] + board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) == 2))) { return BOOL_TYPE_TRUE; }
        } else if (board->pieceCount[PIECE_TYPE_BLACK_ROOK] == 1 && !board->pieceCount[PIECE_TYPE_WHITE_ROOK]) {
            if ((board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] + board->pieceCount[PIECE_TYPE_BLACK_BISHOP] == 0) && (((board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] + board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) == 1) || ((board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] + board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) == 2))) { return BOOL_TYPE_TRUE; }
        }
    }
  return BOOL_TYPE_FALSE;
}

#define ENDGAME_MAT (1 * g_pieceVal[PIECE_TYPE_WHITE_ROOK] + 2 * g_pieceVal[PIECE_TYPE_WHITE_KNIGHT] + 2 * g_pieceVal[PIECE_TYPE_WHITE_PAWN] + g_pieceVal[PIECE_TYPE_WHITE_KING])

int Evaluate_Position(const ChessBoard *board) {

	ASSERT(Board_Check(board));

	int piece;
	int pieceCount;
	int squareIndex;
	int score = board->material[COLOR_TYPE_WHITE] - board->material[COLOR_TYPE_BLACK];
	
	if(!board->pieceCount[PIECE_TYPE_WHITE_PAWN] && !board->pieceCount[PIECE_TYPE_BLACK_PAWN] && MaterialDraw(board) == BOOL_TYPE_TRUE) {
		return 0;
	}
	
	piece = PIECE_TYPE_WHITE_PAWN;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		score += PawnTable[SQUARE_120_TO_64(squareIndex)];	
		
		if( (g_isolatedMask[SQUARE_120_TO_64(squareIndex)] & board->pawns[COLOR_TYPE_WHITE]) == 0) {
			//printf("PIECE_TYPE_WHITE_PAWN Iso:%s\n",PrSq(squareIndex));
			score += PawnIsolated;
		}
		
		if( (g_whitePassedMask[SQUARE_120_TO_64(squareIndex)] & board->pawns[COLOR_TYPE_BLACK]) == 0) {
			//printf("PIECE_TYPE_WHITE_PAWN Passed:%s\n",PrSq(squareIndex));
			score += PawnPassed[g_ranksBoard[squareIndex]];
		}
		
	}	

	piece = PIECE_TYPE_BLACK_PAWN;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))>=0 && SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))<=63);
		score -= PawnTable[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];	
		
		if( (g_isolatedMask[SQUARE_120_TO_64(squareIndex)] & board->pawns[COLOR_TYPE_BLACK]) == 0) {
			//printf("PIECE_TYPE_BLACK_PAWN Iso:%s\n",PrSq(squareIndex));
			score -= PawnIsolated;
		}
		
		if( (g_blackPassedMask[SQUARE_120_TO_64(squareIndex)] & board->pawns[COLOR_TYPE_WHITE]) == 0) {
			//printf("PIECE_TYPE_BLACK_PAWN Passed:%s\n",PrSq(squareIndex));
			score -= PawnPassed[7 - g_ranksBoard[squareIndex]];
		}
	}	
	
	piece = PIECE_TYPE_WHITE_KNIGHT;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		score += KnightTable[SQUARE_120_TO_64(squareIndex)];
	}	

	piece = PIECE_TYPE_BLACK_KNIGHT;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))>=0 && SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))<=63);
		score -= KnightTable[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];
	}			
	
	piece = PIECE_TYPE_WHITE_BISHOP;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		score += BishopTable[SQUARE_120_TO_64(squareIndex)];
	}	

	piece = PIECE_TYPE_BLACK_BISHOP;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))>=0 && SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))<=63);
		score -= BishopTable[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];
	}	

	piece = PIECE_TYPE_WHITE_ROOK;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		score += RookTable[SQUARE_120_TO_64(squareIndex)];
		
		ASSERT(FileRankValid(g_filesBoard[squareIndex]));
		
		if(!(board->pawns[COLOR_TYPE_BOTH] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score += RookOpenFile;
		} else if(!(board->pawns[COLOR_TYPE_WHITE] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score += RookSemiOpenFile;
		}
	}	

	piece = PIECE_TYPE_BLACK_ROOK;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))>=0 && SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))<=63);
		score -= RookTable[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];
		ASSERT(FileRankValid(g_filesBoard[squareIndex]));
		if(!(board->pawns[COLOR_TYPE_BOTH] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score -= RookOpenFile;
		} else if(!(board->pawns[COLOR_TYPE_BLACK] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score -= RookSemiOpenFile;
		}
	}	
	
	piece = PIECE_TYPE_WHITE_QUEEN;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		ASSERT(FileRankValid(g_filesBoard[squareIndex]));
		if(!(board->pawns[COLOR_TYPE_BOTH] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score += QueenOpenFile;
		} else if(!(board->pawns[COLOR_TYPE_WHITE] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score += QueenSemiOpenFile;
		}
	}	

	piece = PIECE_TYPE_BLACK_QUEEN;	
	for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
		squareIndex = board->pList[piece][pieceCount];
		ASSERT(SqOnBoard(squareIndex));
		ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
		ASSERT(FileRankValid(g_filesBoard[squareIndex]));
		if(!(board->pawns[COLOR_TYPE_BOTH] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score -= QueenOpenFile;
		} else if(!(board->pawns[COLOR_TYPE_BLACK] & g_fileBBMask[g_filesBoard[squareIndex]])) {
			score -= QueenSemiOpenFile;
		}
	}	
	//8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62 
	piece = PIECE_TYPE_WHITE_KING;
	squareIndex = board->pList[piece][0];
	ASSERT(SqOnBoard(squareIndex));
	ASSERT(SQUARE_120_TO_64(squareIndex)>=0 && SQUARE_120_TO_64(squareIndex)<=63);
	
	if( (board->material[COLOR_TYPE_BLACK] <= ENDGAME_MAT) ) {
		score += KingE[SQUARE_120_TO_64(squareIndex)];
	} else {
		score += KingO[SQUARE_120_TO_64(squareIndex)];
	}
	
	piece = PIECE_TYPE_BLACK_KING;
	squareIndex = board->pList[piece][0];
	ASSERT(SqOnBoard(squareIndex));
	ASSERT(SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))>=0 && SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))<=63);
	
	if( (board->material[COLOR_TYPE_WHITE] <= ENDGAME_MAT) ) {
		score -= KingE[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];
	} else {
		score -= KingO[SQUARE_MIRROR_64(SQUARE_120_TO_64(squareIndex))];
	}
	
	if(board->pieceCount[PIECE_TYPE_WHITE_BISHOP] >= 2) score += BishopPair;
	if(board->pieceCount[PIECE_TYPE_BLACK_BISHOP] >= 2) score -= BishopPair;
	
	if(board->side == COLOR_TYPE_WHITE) {
		return score;
	} else {
		return -score;
	}	
}


















