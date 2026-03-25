/**
 * @file board_hashkeys.c
 * @brief Zobrist hash key generation
 * 
 * Zobrist hashing is used to create unique hash keys for chess positions.
 * The hash is calculated incrementally as moves are made/unmade.
 * 
 * Hash includes:
 * - Piece placement (piece type and square)
 * - Side to move
 * - Castling rights
 * - En passant square
 * 
 * This allows fast position lookup in the transposition table and
 * efficient detection of threefold repetition.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"

U64 Board_GeneratePositionKey(const ChessBoard *board) {

	int squareIndex = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	
	// pieces
	for(squareIndex = 0; squareIndex < CHESS_BOARD_SQUARE_NUM; ++squareIndex) {
		piece = board->pieces[squareIndex];
		if(piece!=NO_SQ && piece!=EMPTY && piece != OFFBOARD) {
			ASSERT(piece>=PIECE_TYPE_WHITE_PAWN && piece<=PIECE_TYPE_BLACK_KING);
			finalKey ^= g_pieceKeys[piece][squareIndex];
		}		
	}
	
	if(board->side == COLOR_TYPE_WHITE) {
		finalKey ^= g_sideKey;
	}
		
	if(board->enPas != NO_SQ) {
		ASSERT(board->enPas>=0 && board->enPas<CHESS_BOARD_SQUARE_NUM);
		ASSERT(SqOnBoard(board->enPas));
		ASSERT(g_ranksBoard[board->enPas] == RANK_TYPE_3 || g_ranksBoard[board->enPas] == RANK_TYPE_6);
		finalKey ^= g_pieceKeys[EMPTY][board->enPas];
	}
	
	ASSERT(board->castlePerm>=0 && board->castlePerm<=15);
	
	finalKey ^= g_castleKeys[board->castlePerm];
	
	return finalKey;
}

