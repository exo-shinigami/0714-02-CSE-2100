/**
 * @file moves_generation.c
 * @brief Move generation functions
 * 
 * Generates all pseudo-legal moves for a given position.
 * Includes specialized generation for:
 * - Pawn moves (including double pushes, promotions, en passant)
 * - Knight moves
 * - Sliding piece moves (bishops, rooks, queens)
 * - King moves (including castling)
 * 
 * Also implements:
 * - Capture-only move generation (for quiescence search)
 * - Move ordering using MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
 * - Move validation
 * 
 * Note: Generated moves are pseudo-legal and must be validated
 * with Move_Make() to ensure they don't leave the king in check.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

// movegen.c

#include "stdio.h"
#include "types_definitions.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(squareIndex) (g_filesBoard[(squareIndex)]==OFFBOARD)

const int LoopSlidePce[8] = {
 PIECE_TYPE_WHITE_BISHOP, PIECE_TYPE_WHITE_ROOK, PIECE_TYPE_WHITE_QUEEN, 0, PIECE_TYPE_BLACK_BISHOP, PIECE_TYPE_BLACK_ROOK, PIECE_TYPE_BLACK_QUEEN, 0
};

const int LoopNonSlidePce[6] = {
 PIECE_TYPE_WHITE_KNIGHT, PIECE_TYPE_WHITE_KING, 0, PIECE_TYPE_BLACK_KNIGHT, PIECE_TYPE_BLACK_KING, 0
};

const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };

const int PceDir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

const int NumDir[13] = {
 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

/*
PV Move
capturedPiece -> MvvLVA
Killers
HistoryScore

*/
const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void Init_MvvLva() {
	int Attacker;
	int Victim;
	for(Attacker = PIECE_TYPE_WHITE_PAWN; Attacker <= PIECE_TYPE_BLACK_KING; ++Attacker) {
		for(Victim = PIECE_TYPE_WHITE_PAWN; Victim <= PIECE_TYPE_BLACK_KING; ++Victim) {
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
		}
	}
}

int MoveExists(ChessBoard *board, const int move) {

	MoveList list[1];
    Move_GenerateAll(board,list);

    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if ( !Move_Make(board,list->moves[MoveNum].move))  {
            continue;
        }
        Move_Take(board);
		if(list->moves[MoveNum].move == move) {
			return BOOL_TYPE_TRUE;
		}
    }
	return BOOL_TYPE_FALSE;
}

static void AddQuietMove( const ChessBoard *board, int move, MoveList *list ) {

	ASSERT(SqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(SqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(Board_Check(board));
	ASSERT(board->ply >=0 && board->ply < CHESS_MAX_SEARCH_DEPTH);

	list->moves[list->count].move = move;

	if(board->searchKillers[0][board->ply] == move) {
		list->moves[list->count].score = 900000;
	} else if(board->searchKillers[1][board->ply] == move) {
		list->moves[list->count].score = 800000;
	} else {
		list->moves[list->count].score = board->searchHistory[board->pieces[MOVE_GET_FROM_SQUARE(move)]][MOVE_GET_TO_SQUARE(move)];
	}
	list->count++;
}

static void AddCaptureMove( const ChessBoard *board, int move, MoveList *list ) {

	ASSERT(SqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(SqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(PieceValid(MOVE_GET_CAPTURED(move)));
	ASSERT(Board_Check(board));

	list->moves[list->count].move = move;
	list->moves[list->count].score = MvvLvaScores[MOVE_GET_CAPTURED(move)][board->pieces[MOVE_GET_FROM_SQUARE(move)]] + 1000000;
	list->count++;
}

static void AddEnPassantMove( const ChessBoard *board, int move, MoveList *list ) {

	ASSERT(SqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(SqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(Board_Check(board));
	ASSERT((g_ranksBoard[MOVE_GET_TO_SQUARE(move)]==RANK_TYPE_6 && board->side == COLOR_TYPE_WHITE) || (g_ranksBoard[MOVE_GET_TO_SQUARE(move)]==RANK_TYPE_3 && board->side == COLOR_TYPE_BLACK));

	list->moves[list->count].move = move;
	list->moves[list->count].score = 105 + 1000000;
	list->count++;
}

static void AddWhitePawnCapMove( const ChessBoard *board, const int from, const int to, const int capturedPiece, MoveList *list ) {

	ASSERT(PieceValidEmpty(capturedPiece));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(Board_Check(board));

	if(g_ranksBoard[from] == RANK_TYPE_7) {
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_QUEEN,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_ROOK,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_BISHOP,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_KNIGHT,0), list);
	} else {
		AddCaptureMove(board, MOVE(from,to,capturedPiece,EMPTY,0), list);
	}
}

static void AddWhitePawnMove( const ChessBoard *board, const int from, const int to, MoveList *list ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(Board_Check(board));

	if(g_ranksBoard[from] == RANK_TYPE_7) {
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_QUEEN,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_ROOK,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_BISHOP,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_KNIGHT,0), list);
	} else {
		AddQuietMove(board, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

static void AddBlackPawnCapMove( const ChessBoard *board, const int from, const int to, const int capturedPiece, MoveList *list ) {

	ASSERT(PieceValidEmpty(capturedPiece));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(Board_Check(board));

	if(g_ranksBoard[from] == RANK_TYPE_2) {
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_QUEEN,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_ROOK,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_BISHOP,0), list);
		AddCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_KNIGHT,0), list);
	} else {
		AddCaptureMove(board, MOVE(from,to,capturedPiece,EMPTY,0), list);
	}
}

static void AddBlackPawnMove( const ChessBoard *board, const int from, const int to, MoveList *list ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(Board_Check(board));

	if(g_ranksBoard[from] == RANK_TYPE_2) {
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_QUEEN,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_ROOK,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_BISHOP,0), list);
		AddQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_KNIGHT,0), list);
	} else {
		AddQuietMove(board, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

void Move_GenerateAll(const ChessBoard *board, MoveList *list) {

	ASSERT(Board_Check(board));

	list->count = 0;

	int piece = EMPTY;
	int side = board->side;
	int squareIndex = 0; int t_sq = 0;
	int pieceCount = 0;
	int direction = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == COLOR_TYPE_WHITE) {

		for(pieceCount = 0; pieceCount < board->pieceCount[PIECE_TYPE_WHITE_PAWN]; ++pieceCount) {
			squareIndex = board->pList[PIECE_TYPE_WHITE_PAWN][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			if(board->pieces[squareIndex + 10] == EMPTY) {
				AddWhitePawnMove(board, squareIndex, squareIndex+10, list);
				if(g_ranksBoard[squareIndex] == RANK_TYPE_2 && board->pieces[squareIndex + 20] == EMPTY) {
					AddQuietMove(board, MOVE(squareIndex,(squareIndex+20),EMPTY,EMPTY,MFLAGPS),list);
				}
			}

			if(!SQOFFBOARD(squareIndex + 9) && g_pieceCol[board->pieces[squareIndex + 9]] == COLOR_TYPE_BLACK) {
				AddWhitePawnCapMove(board, squareIndex, squareIndex+9, board->pieces[squareIndex + 9], list);
			}
			if(!SQOFFBOARD(squareIndex + 11) && g_pieceCol[board->pieces[squareIndex + 11]] == COLOR_TYPE_BLACK) {
				AddWhitePawnCapMove(board, squareIndex, squareIndex+11, board->pieces[squareIndex + 11], list);
			}

			if(board->enPas != NO_SQ) {
				if(squareIndex + 9 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex + 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(squareIndex + 11 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex + 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}

		if(board->castlePerm & CASTLE_TYPE_WKCA) {
			if(board->pieces[F1] == EMPTY && board->pieces[G1] == EMPTY) {
				if(!Attack_IsSquareAttacked(E1,COLOR_TYPE_BLACK,board) && !Attack_IsSquareAttacked(F1,COLOR_TYPE_BLACK,board) ) {
					AddQuietMove(board, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		if(board->castlePerm & CASTLE_TYPE_WQCA) {
			if(board->pieces[D1] == EMPTY && board->pieces[C1] == EMPTY && board->pieces[B1] == EMPTY) {
				if(!Attack_IsSquareAttacked(E1,COLOR_TYPE_BLACK,board) && !Attack_IsSquareAttacked(D1,COLOR_TYPE_BLACK,board) ) {
					AddQuietMove(board, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < board->pieceCount[PIECE_TYPE_BLACK_PAWN]; ++pieceCount) {
			squareIndex = board->pList[PIECE_TYPE_BLACK_PAWN][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			if(board->pieces[squareIndex - 10] == EMPTY) {
				AddBlackPawnMove(board, squareIndex, squareIndex-10, list);
				if(g_ranksBoard[squareIndex] == RANK_TYPE_7 && board->pieces[squareIndex - 20] == EMPTY) {
					AddQuietMove(board, MOVE(squareIndex,(squareIndex-20),EMPTY,EMPTY,MFLAGPS),list);
				}
			}

			if(!SQOFFBOARD(squareIndex - 9) && g_pieceCol[board->pieces[squareIndex - 9]] == COLOR_TYPE_WHITE) {
				AddBlackPawnCapMove(board, squareIndex, squareIndex-9, board->pieces[squareIndex - 9], list);
			}

			if(!SQOFFBOARD(squareIndex - 11) && g_pieceCol[board->pieces[squareIndex - 11]] == COLOR_TYPE_WHITE) {
				AddBlackPawnCapMove(board, squareIndex, squareIndex-11, board->pieces[squareIndex - 11], list);
			}
			if(board->enPas != NO_SQ) {
				if(squareIndex - 9 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex - 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(squareIndex - 11 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex - 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}

		// castling
		if(board->castlePerm &  CASTLE_TYPE_BKCA) {
			if(board->pieces[F8] == EMPTY && board->pieces[G8] == EMPTY) {
				if(!Attack_IsSquareAttacked(E8,COLOR_TYPE_WHITE,board) && !Attack_IsSquareAttacked(F8,COLOR_TYPE_WHITE,board) ) {
					AddQuietMove(board, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		if(board->castlePerm &  CASTLE_TYPE_BQCA) {
			if(board->pieces[D8] == EMPTY && board->pieces[C8] == EMPTY && board->pieces[B8] == EMPTY) {
				if(!Attack_IsSquareAttacked(E8,COLOR_TYPE_WHITE,board) && !Attack_IsSquareAttacked(D8,COLOR_TYPE_WHITE,board) ) {
					AddQuietMove(board, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	piece = LoopSlidePce[pceIndex++];
	while( piece != 0) {
		ASSERT(PieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
			squareIndex = board->pList[piece][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				while(!SQOFFBOARD(t_sq)) {
					// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
					if(board->pieces[t_sq] != EMPTY) {
						if( g_pieceCol[board->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(board, MOVE(squareIndex, t_sq, board->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					AddQuietMove(board, MOVE(squareIndex, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += direction;
				}
			}
		}

		piece = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	piece = LoopNonSlidePce[pceIndex++];

	while( piece != 0) {
		ASSERT(PieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
			squareIndex = board->pList[piece][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
				if(board->pieces[t_sq] != EMPTY) {
					if( g_pieceCol[board->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(board, MOVE(squareIndex, t_sq, board->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				AddQuietMove(board, MOVE(squareIndex, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		piece = LoopNonSlidePce[pceIndex++];
	}

    ASSERT(MoveListOk(list,board));
}


void GenerateAllCaps(const ChessBoard *board, MoveList *list) {

	ASSERT(Board_Check(board));

	list->count = 0;

	int piece = EMPTY;
	int side = board->side;
	int squareIndex = 0; int t_sq = 0;
	int pieceCount = 0;
	int direction = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == COLOR_TYPE_WHITE) {

		for(pieceCount = 0; pieceCount < board->pieceCount[PIECE_TYPE_WHITE_PAWN]; ++pieceCount) {
			squareIndex = board->pList[PIECE_TYPE_WHITE_PAWN][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			if(!SQOFFBOARD(squareIndex + 9) && g_pieceCol[board->pieces[squareIndex + 9]] == COLOR_TYPE_BLACK) {
				AddWhitePawnCapMove(board, squareIndex, squareIndex+9, board->pieces[squareIndex + 9], list);
			}
			if(!SQOFFBOARD(squareIndex + 11) && g_pieceCol[board->pieces[squareIndex + 11]] == COLOR_TYPE_BLACK) {
				AddWhitePawnCapMove(board, squareIndex, squareIndex+11, board->pieces[squareIndex + 11], list);
			}

			if(board->enPas != NO_SQ) {
				if(squareIndex + 9 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex + 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(squareIndex + 11 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex + 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < board->pieceCount[PIECE_TYPE_BLACK_PAWN]; ++pieceCount) {
			squareIndex = board->pList[PIECE_TYPE_BLACK_PAWN][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			if(!SQOFFBOARD(squareIndex - 9) && g_pieceCol[board->pieces[squareIndex - 9]] == COLOR_TYPE_WHITE) {
				AddBlackPawnCapMove(board, squareIndex, squareIndex-9, board->pieces[squareIndex - 9], list);
			}

			if(!SQOFFBOARD(squareIndex - 11) && g_pieceCol[board->pieces[squareIndex - 11]] == COLOR_TYPE_WHITE) {
				AddBlackPawnCapMove(board, squareIndex, squareIndex-11, board->pieces[squareIndex - 11], list);
			}
			if(board->enPas != NO_SQ) {
				if(squareIndex - 9 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex - 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(squareIndex - 11 == board->enPas) {
					AddEnPassantMove(board, MOVE(squareIndex,squareIndex - 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	piece = LoopSlidePce[pceIndex++];
	while( piece != 0) {
		ASSERT(PieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
			squareIndex = board->pList[piece][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				while(!SQOFFBOARD(t_sq)) {
					// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
					if(board->pieces[t_sq] != EMPTY) {
						if( g_pieceCol[board->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(board, MOVE(squareIndex, t_sq, board->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += direction;
				}
			}
		}

		piece = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	piece = LoopNonSlidePce[pceIndex++];

	while( piece != 0) {
		ASSERT(PieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCount[piece]; ++pieceCount) {
			squareIndex = board->pList[piece][pieceCount];
			ASSERT(SqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
				if(board->pieces[t_sq] != EMPTY) {
					if( g_pieceCol[board->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(board, MOVE(squareIndex, t_sq, board->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
			}
		}

		piece = LoopNonSlidePce[pceIndex++];
	}
    ASSERT(MoveListOk(list,board));
}




