/**
 * @file moves_generation.c
 * @brief move generation functions
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
 * - move ordering using MVV-LVA (Most Valuable victim - Least Valuable attacker)
 * - move validation
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
PV move
capturedPiece -> MvvLVA
Killers
HistoryScore

*/
const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void initMvvLva() {
	int attacker;
	int victim;
	for(attacker = PIECE_TYPE_WHITE_PAWN; attacker <= PIECE_TYPE_BLACK_KING; ++attacker) {
		for(victim = PIECE_TYPE_WHITE_PAWN; victim <= PIECE_TYPE_BLACK_KING; ++victim) {
			MvvLvaScores[victim][attacker] = VictimScore[victim] + 6 - ( VictimScore[attacker] / 100);
		}
	}
}

int moveExists(ChessBoard *board, const int move) {

	MoveList list[1];
    moveGenerateAll(board,list);

    int moveNum = 0;
	for(moveNum = 0; moveNum < list->size(); ++moveNum) {

        if ( !board->makeMove(list->at(moveNum).raw()))  {
            continue;
        }
        board->takeMove();
		if(list->at(moveNum).raw() == move) {
			return BOOL_TYPE_TRUE;
		}
    }
	return BOOL_TYPE_FALSE;
}

static void addQuietMove( const ChessBoard *board, int move, MoveList *list, const SearchInfo *info ) {

	ASSERT(sqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(sqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(board->check());
	ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);

	int score = 0;
	if (info) {
		if(info->searchKillers[0][board->getPly()] == move) {
			score = 900000;
		} else if(info->searchKillers[1][board->getPly()] == move) {
			score = 800000;
		} else {
			score = info->searchHistory[board->pieceAt(MOVE_GET_FROM_SQUARE(move))][MOVE_GET_TO_SQUARE(move)];
		}
	}
	list->push(move, score);
}

static void addCaptureMove( const ChessBoard *board, int move, MoveList *list, const SearchInfo * ) {

	ASSERT(sqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(sqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(pieceValid(MOVE_GET_CAPTURED(move)));
	ASSERT(board->check());

	list->push(move, MvvLvaScores[MOVE_GET_CAPTURED(move)][board->pieceAt(MOVE_GET_FROM_SQUARE(move))] + 1000000);
}

static void addEnPassantMove( const ChessBoard *board, int move, MoveList *list, const SearchInfo * ) {

	ASSERT(sqOnBoard(MOVE_GET_FROM_SQUARE(move)));
	ASSERT(sqOnBoard(MOVE_GET_TO_SQUARE(move)));
	ASSERT(board->check());
	ASSERT((g_ranksBoard[MOVE_GET_TO_SQUARE(move)]==RANK_TYPE_6 && board->getSide() == COLOR_TYPE_WHITE) || (g_ranksBoard[MOVE_GET_TO_SQUARE(move)]==RANK_TYPE_3 && board->getSide() == COLOR_TYPE_BLACK));

	list->push(move, 105 + 1000000);
}

static void addWhitePawnCapMove( const ChessBoard *board, const int from, const int to, const int capturedPiece, MoveList *list, const SearchInfo *info ) {

	ASSERT(pieceValidEmpty(capturedPiece));
	ASSERT(sqOnBoard(from));
	ASSERT(sqOnBoard(to));
	ASSERT(board->check());

	if(g_ranksBoard[from] == RANK_TYPE_7) {
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_QUEEN,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_ROOK,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_BISHOP,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_WHITE_KNIGHT,0), list, info);
	} else {
		addCaptureMove(board, MOVE(from,to,capturedPiece,EMPTY,0), list, info);
	}
}

static void addWhitePawnMove( const ChessBoard *board, const int from, const int to, MoveList *list, const SearchInfo *info ) {

	ASSERT(sqOnBoard(from));
	ASSERT(sqOnBoard(to));
	ASSERT(board->check());

	if(g_ranksBoard[from] == RANK_TYPE_7) {
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_QUEEN,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_ROOK,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_BISHOP,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_WHITE_KNIGHT,0), list, info);
	} else {
		addQuietMove(board, MOVE(from,to,EMPTY,EMPTY,0), list, info);
	}
}

static void addBlackPawnCapMove( const ChessBoard *board, const int from, const int to, const int capturedPiece, MoveList *list, const SearchInfo *info ) {

	ASSERT(pieceValidEmpty(capturedPiece));
	ASSERT(sqOnBoard(from));
	ASSERT(sqOnBoard(to));
	ASSERT(board->check());

	if(g_ranksBoard[from] == RANK_TYPE_2) {
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_QUEEN,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_ROOK,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_BISHOP,0), list, info);
		addCaptureMove(board, MOVE(from,to,capturedPiece,PIECE_TYPE_BLACK_KNIGHT,0), list, info);
	} else {
		addCaptureMove(board, MOVE(from,to,capturedPiece,EMPTY,0), list, info);
	}
}

static void addBlackPawnMove( const ChessBoard *board, const int from, const int to, MoveList *list, const SearchInfo *info ) {

	ASSERT(sqOnBoard(from));
	ASSERT(sqOnBoard(to));
	ASSERT(board->check());

	if(g_ranksBoard[from] == RANK_TYPE_2) {
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_QUEEN,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_ROOK,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_BISHOP,0), list, info);
		addQuietMove(board, MOVE(from,to,EMPTY,PIECE_TYPE_BLACK_KNIGHT,0), list, info);
	} else {
		addQuietMove(board, MOVE(from,to,EMPTY,EMPTY,0), list, info);
	}
}

void moveGenerateAll(const ChessBoard *board, MoveList *list, const SearchInfo *info) {

	ASSERT(board->check());

	list->clear();

	int piece = EMPTY;
	int side = board->getSide();
	int squareIndex = 0; int t_sq = 0;
	int pieceCount = 0;
	int direction = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == COLOR_TYPE_WHITE) {

		for(pieceCount = 0; pieceCount < board->pieceCountAt(PIECE_TYPE_WHITE_PAWN); ++pieceCount) {
			squareIndex = board->pieceListAt(PIECE_TYPE_WHITE_PAWN, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			if(board->pieceAt(squareIndex + 10) == EMPTY) {
				addWhitePawnMove(board, squareIndex, squareIndex+10, list, info);
				if(g_ranksBoard[squareIndex] == RANK_TYPE_2 && board->pieceAt(squareIndex + 20) == EMPTY) {
					addQuietMove(board, MOVE(squareIndex,(squareIndex+20),EMPTY,EMPTY,MFLAGPS),list, info);
				}
			}

			if(!SQOFFBOARD(squareIndex + 9) && g_pieceCol[board->pieceAt(squareIndex + 9)] == COLOR_TYPE_BLACK) {
				addWhitePawnCapMove(board, squareIndex, squareIndex+9, board->pieceAt(squareIndex + 9), list, info);
			}
			if(!SQOFFBOARD(squareIndex + 11) && g_pieceCol[board->pieceAt(squareIndex + 11)] == COLOR_TYPE_BLACK) {
				addWhitePawnCapMove(board, squareIndex, squareIndex+11, board->pieceAt(squareIndex + 11), list, info);
			}

			if(board->getEnPassantSquare() != NO_SQ) {
				if(squareIndex + 9 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex + 9,EMPTY,EMPTY,MFLAGEP), list, info);
				}
				if(squareIndex + 11 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex + 11,EMPTY,EMPTY,MFLAGEP), list, info);
				}
			}
		}

		if(board->getCastlePermission() & CASTLE_TYPE_WKCA) {
			if(board->pieceAt(F1) == EMPTY && board->pieceAt(G1) == EMPTY) {
				if(!board->isSquareAttacked(E1, COLOR_TYPE_BLACK) && !board->isSquareAttacked(F1, COLOR_TYPE_BLACK) ) {
					addQuietMove(board, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list, info);
				}
			}
		}

		if(board->getCastlePermission() & CASTLE_TYPE_WQCA) {
			if(board->pieceAt(D1) == EMPTY && board->pieceAt(C1) == EMPTY && board->pieceAt(B1) == EMPTY) {
				if(!board->isSquareAttacked(E1, COLOR_TYPE_BLACK) && !board->isSquareAttacked(D1, COLOR_TYPE_BLACK) ) {
					addQuietMove(board, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list, info);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < board->pieceCountAt(PIECE_TYPE_BLACK_PAWN); ++pieceCount) {
			squareIndex = board->pieceListAt(PIECE_TYPE_BLACK_PAWN, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			if(board->pieceAt(squareIndex - 10) == EMPTY) {
				addBlackPawnMove(board, squareIndex, squareIndex-10, list, info);
				if(g_ranksBoard[squareIndex] == RANK_TYPE_7 && board->pieceAt(squareIndex - 20) == EMPTY) {
					addQuietMove(board, MOVE(squareIndex,(squareIndex-20),EMPTY,EMPTY,MFLAGPS),list, info);
				}
			}

			if(!SQOFFBOARD(squareIndex - 9) && g_pieceCol[board->pieceAt(squareIndex - 9)] == COLOR_TYPE_WHITE) {
				addBlackPawnCapMove(board, squareIndex, squareIndex-9, board->pieceAt(squareIndex - 9), list, info);
			}

			if(!SQOFFBOARD(squareIndex - 11) && g_pieceCol[board->pieceAt(squareIndex - 11)] == COLOR_TYPE_WHITE) {
				addBlackPawnCapMove(board, squareIndex, squareIndex-11, board->pieceAt(squareIndex - 11), list, info);
			}
			if(board->getEnPassantSquare() != NO_SQ) {
				if(squareIndex - 9 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex - 9,EMPTY,EMPTY,MFLAGEP), list, info);
				}
				if(squareIndex - 11 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex - 11,EMPTY,EMPTY,MFLAGEP), list, info);
				}
			}
		}

		// castling
		if(board->getCastlePermission() &  CASTLE_TYPE_BKCA) {
			if(board->pieceAt(F8) == EMPTY && board->pieceAt(G8) == EMPTY) {
				if(!board->isSquareAttacked(E8, COLOR_TYPE_WHITE) && !board->isSquareAttacked(F8, COLOR_TYPE_WHITE) ) {
					addQuietMove(board, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list, info);
				}
			}
		}

		if(board->getCastlePermission() &  CASTLE_TYPE_BQCA) {
			if(board->pieceAt(D8) == EMPTY && board->pieceAt(C8) == EMPTY && board->pieceAt(B8) == EMPTY) {
				if(!board->isSquareAttacked(E8, COLOR_TYPE_WHITE) && !board->isSquareAttacked(D8, COLOR_TYPE_WHITE) ) {
					addQuietMove(board, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list, info);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	piece = LoopSlidePce[pceIndex++];
	while( piece != 0) {
		ASSERT(pieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCountAt(piece); ++pieceCount) {
			squareIndex = board->pieceListAt(piece, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				while(!SQOFFBOARD(t_sq)) {
					// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
					if(board->pieceAt(t_sq) != EMPTY) {
						if( g_pieceCol[board->pieceAt(t_sq)] == (side ^ 1)) {
						addCaptureMove(board, MOVE(squareIndex, t_sq, board->pieceAt(t_sq), EMPTY, 0), list, info);
					}
					break;
				}
				addQuietMove(board, MOVE(squareIndex, t_sq, EMPTY, EMPTY, 0), list, info);
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
		ASSERT(pieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCountAt(piece); ++pieceCount) {
			squareIndex = board->pieceListAt(piece, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
				if(board->pieceAt(t_sq) != EMPTY) {
					if( g_pieceCol[board->pieceAt(t_sq)] == (side ^ 1)) {
						addCaptureMove(board, MOVE(squareIndex, t_sq, board->pieceAt(t_sq), EMPTY, 0), list, info);
					}
					continue;
				}
				addQuietMove(board, MOVE(squareIndex, t_sq, EMPTY, EMPTY, 0), list, info);
			}
		}

		piece = LoopNonSlidePce[pceIndex++];
	}

    ASSERT(moveListOk(list,board));
}


void generateAllCaps(const ChessBoard *board, MoveList *list, const SearchInfo *info) {
	ASSERT(board->check());

	list->clear();

	int piece = EMPTY;
	int side = board->getSide();
	int squareIndex = 0; int t_sq = 0;
	int pieceCount = 0;
	int direction = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == COLOR_TYPE_WHITE) {

		for(pieceCount = 0; pieceCount < board->pieceCountAt(PIECE_TYPE_WHITE_PAWN); ++pieceCount) {
			squareIndex = board->pieceListAt(PIECE_TYPE_WHITE_PAWN, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			if(!SQOFFBOARD(squareIndex + 9) && g_pieceCol[board->pieceAt(squareIndex + 9)] == COLOR_TYPE_BLACK) {
				addWhitePawnCapMove(board, squareIndex, squareIndex+9, board->pieceAt(squareIndex + 9), list, info);
			}
			if(!SQOFFBOARD(squareIndex + 11) && g_pieceCol[board->pieceAt(squareIndex + 11)] == COLOR_TYPE_BLACK) {
				addWhitePawnCapMove(board, squareIndex, squareIndex+11, board->pieceAt(squareIndex + 11), list, info);
			}

			if(board->getEnPassantSquare() != NO_SQ) {
				if(squareIndex + 9 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex + 9,EMPTY,EMPTY,MFLAGEP), list, info);
				}
				if(squareIndex + 11 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex + 11,EMPTY,EMPTY,MFLAGEP), list, info);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < board->pieceCountAt(PIECE_TYPE_BLACK_PAWN); ++pieceCount) {
			squareIndex = board->pieceListAt(PIECE_TYPE_BLACK_PAWN, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			if(!SQOFFBOARD(squareIndex - 9) && g_pieceCol[board->pieceAt(squareIndex - 9)] == COLOR_TYPE_WHITE) {
				addBlackPawnCapMove(board, squareIndex, squareIndex-9, board->pieceAt(squareIndex - 9), list, info);
			}

			if(!SQOFFBOARD(squareIndex - 11) && g_pieceCol[board->pieceAt(squareIndex - 11)] == COLOR_TYPE_WHITE) {
				addBlackPawnCapMove(board, squareIndex, squareIndex-11, board->pieceAt(squareIndex - 11), list, info);
			}
			if(board->getEnPassantSquare() != NO_SQ) {
				if(squareIndex - 9 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex - 9,EMPTY,EMPTY,MFLAGEP), list, info);
				}
				if(squareIndex - 11 == board->getEnPassantSquare()) {
					addEnPassantMove(board, MOVE(squareIndex,squareIndex - 11,EMPTY,EMPTY,MFLAGEP), list, info);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	piece = LoopSlidePce[pceIndex++];
	while( piece != 0) {
		ASSERT(pieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCountAt(piece); ++pieceCount) {
			squareIndex = board->pieceListAt(piece, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				while(!SQOFFBOARD(t_sq)) {
					// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
					if(board->pieceAt(t_sq) != EMPTY) {
						if( g_pieceCol[board->pieceAt(t_sq)] == (side ^ 1)) {
							addCaptureMove(board, MOVE(squareIndex, t_sq, board->pieceAt(t_sq), EMPTY, 0), list, info);
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
		ASSERT(pieceValid(piece));

		for(pieceCount = 0; pieceCount < board->pieceCountAt(piece); ++pieceCount) {
			squareIndex = board->pieceListAt(piece, pieceCount);
			ASSERT(sqOnBoard(squareIndex));

			for(index = 0; index < NumDir[piece]; ++index) {
				direction = PceDir[piece][index];
				t_sq = squareIndex + direction;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// COLOR_TYPE_BLACK ^ 1 == COLOR_TYPE_WHITE       COLOR_TYPE_WHITE ^ 1 == COLOR_TYPE_BLACK
				if(board->pieceAt(t_sq) != EMPTY) {
					if( g_pieceCol[board->pieceAt(t_sq)] == (side ^ 1)) {
						addCaptureMove(board, MOVE(squareIndex, t_sq, board->pieceAt(t_sq), EMPTY, 0), list, info);
					}
					continue;
				}
			}
		}

		piece = LoopNonSlidePce[pceIndex++];
	}
    ASSERT(moveListOk(list,board));
}




