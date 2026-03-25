/**
 * @file attack_detection.c
 * @brief Square attack detection functions
 * 
 * Determines if a given square is under attack by pieces of a specific color.
 * This is crucial for:
 * - Check detection
 * - Move legality validation
 * - Castling rights verification
 * - King safety evaluation
 * 
 * Uses efficient sliding piece attack generation with direction vectors
 * and precomputed knight/king attack patterns.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"

const int KnDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,	1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

int Attack_IsSquareAttacked(const int squareIndex, const int side, const ChessBoard *board) {

	int piece,index,t_sq,direction;
	
	ASSERT(SqOnBoard(squareIndex));
	ASSERT(SideValid(side));
	ASSERT(Board_Check(board));
	
	// pawns
	if(side == COLOR_TYPE_WHITE) {
		if(board->pieces[squareIndex-11] == PIECE_TYPE_WHITE_PAWN || board->pieces[squareIndex-9] == PIECE_TYPE_WHITE_PAWN) {
			return BOOL_TYPE_TRUE;
		}
	} else {
		if(board->pieces[squareIndex+11] == PIECE_TYPE_BLACK_PAWN || board->pieces[squareIndex+9] == PIECE_TYPE_BLACK_PAWN) {
			return BOOL_TYPE_TRUE;
		}	
	}
	
	// knights
	for(index = 0; index < 8; ++index) {		
		piece = board->pieces[squareIndex + KnDir[index]];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece != OFFBOARD && PIECE_IS_KNIGHT(piece) && g_pieceCol[piece]==side) {
			return BOOL_TYPE_TRUE;
		}
	}
	
	// rooks, queens
	for(index = 0; index < 4; ++index) {		
		direction = RkDir[index];
		t_sq = squareIndex + direction;
		ASSERT(SqIs120(t_sq));
		piece = board->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(piece));
		while(piece != OFFBOARD) {
			if(piece != EMPTY) {
				if(PIECE_IS_ROOK_QUEEN(piece) && g_pieceCol[piece] == side) {
					return BOOL_TYPE_TRUE;
				}
				break;
			}
			t_sq += direction;
			ASSERT(SqIs120(t_sq));
			piece = board->pieces[t_sq];
		}
	}
	
	// bishops, queens
	for(index = 0; index < 4; ++index) {		
		direction = BiDir[index];
		t_sq = squareIndex + direction;
		ASSERT(SqIs120(t_sq));
		piece = board->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(piece));
		while(piece != OFFBOARD) {
			if(piece != EMPTY) {
				if(PIECE_IS_BISHOP_QUEEN(piece) && g_pieceCol[piece] == side) {
					return BOOL_TYPE_TRUE;
				}
				break;
			}
			t_sq += direction;
			ASSERT(SqIs120(t_sq));
			piece = board->pieces[t_sq];
		}
	}
	
	// kings
	for(index = 0; index < 8; ++index) {		
		piece = board->pieces[squareIndex + KiDir[index]];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece != OFFBOARD && PIECE_IS_KING(piece) && g_pieceCol[piece]==side) {
			return BOOL_TYPE_TRUE;
		}
	}
	
	return BOOL_TYPE_FALSE;
	
}