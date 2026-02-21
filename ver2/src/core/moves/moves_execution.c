/**
 * @file moves_execution.c
 * @brief Move execution and retraction functions
 * 
 * Handles making and unmaking moves on the board, including:
 * - Regular moves and captures
 * - Castle moves
 * - En passant captures
 * - Pawn promotions
 * - Null moves (for null move pruning)
 * 
 * Key functions maintain board consistency by:
 * - Updating piece arrays and bitboards
 * - Maintaining piece lists
 * - Updating material counts
 * - Recalculating hash keys
 * - Validating move legality (no self-check)
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "types_definitions.h"
#include "stdio.h"

#define HASH_PCE(piece,squareIndex) (board->posKey ^= (g_pieceKeys[(piece)][(squareIndex)]))
#define HASH_CA (board->posKey ^= (g_castleKeys[(board->castlePerm)]))
#define HASH_SIDE (board->posKey ^= (g_sideKey))
#define HASH_EP (board->posKey ^= (g_pieceKeys[EMPTY][(board->enPas)]))

const int CastlePerm[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void ClearPiece(const int squareIndex, ChessBoard *board) {

	ASSERT(SqOnBoard(squareIndex));
	ASSERT(Board_Check(board));
	
    int piece = board->pieces[squareIndex];
	
    ASSERT(PieceValid(piece));
	
	int color = g_pieceCol[piece];
	int index = 0;
	int t_pceNum = -1;
	
	ASSERT(SideValid(color));
	
    HASH_PCE(piece,squareIndex);
	
	board->pieces[squareIndex] = EMPTY;
    board->material[color] -= g_pieceVal[piece];
	
	if(PieceBig[piece]) {
			board->bigPce[color]--;
		if(PieceMaj[piece]) {
			board->majPce[color]--;
		} else {
			board->minPce[color]--;
		}
	} else {
		BITBOARD_CLEAR_BIT(board->pawns[color],SQUARE_120_TO_64(squareIndex));
		BITBOARD_CLEAR_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(squareIndex));
	}
	
	for(index = 0; index < board->pieceCount[piece]; ++index) {
		if(board->pList[piece][index] == squareIndex) {
			t_pceNum = index;
			break;
		}
	}
	
	ASSERT(t_pceNum != -1);
	ASSERT(t_pceNum>=0&&t_pceNum<10);
	
	board->pieceCount[piece]--;		
	
	board->pList[piece][t_pceNum] = board->pList[piece][board->pieceCount[piece]];
  
}


static void AddPiece(const int squareIndex, ChessBoard *board, const int piece) {

    ASSERT(PieceValid(piece));
    ASSERT(SqOnBoard(squareIndex));
	
	int color = g_pieceCol[piece];
	ASSERT(SideValid(color));

    HASH_PCE(piece,squareIndex);
	
	board->pieces[squareIndex] = piece;

    if(PieceBig[piece]) {
			board->bigPce[color]++;
		if(PieceMaj[piece]) {
			board->majPce[color]++;
		} else {
			board->minPce[color]++;
		}
	} else {
		BITBOARD_SET_BIT(board->pawns[color],SQUARE_120_TO_64(squareIndex));
		BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(squareIndex));
	}
	
	board->material[color] += g_pieceVal[piece];
	board->pList[piece][board->pieceCount[piece]++] = squareIndex;
	
}

static void MovePiece(const int from, const int to, ChessBoard *board) {

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
	
	int index = 0;
	int piece = board->pieces[from];	
	int color = g_pieceCol[piece];
	ASSERT(SideValid(color));
    ASSERT(PieceValid(piece));
	
#ifdef DEBUG
	int t_PieceNum = BOOL_TYPE_FALSE;
#endif

	HASH_PCE(piece,from);
	board->pieces[from] = EMPTY;
	
	HASH_PCE(piece,to);
	board->pieces[to] = piece;
	
	if(!PieceBig[piece]) {
		BITBOARD_CLEAR_BIT(board->pawns[color],SQUARE_120_TO_64(from));
		BITBOARD_CLEAR_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(from));
		BITBOARD_SET_BIT(board->pawns[color],SQUARE_120_TO_64(to));
		BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(to));		
	}    
	
	for(index = 0; index < board->pieceCount[piece]; ++index) {
		if(board->pList[piece][index] == from) {
			board->pList[piece][index] = to;
#ifdef DEBUG
			t_PieceNum = BOOL_TYPE_TRUE;
#endif
			break;
		}
	}
	ASSERT(t_PieceNum);
}

int Move_Make(ChessBoard *board, int move) {

	ASSERT(Board_Check(board));
	
	int from = MOVE_GET_FROM_SQUARE(move);
    int to = MOVE_GET_TO_SQUARE(move);
    int side = board->side;
	
	ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(SideValid(side));
    ASSERT(PieceValid(board->pieces[from]));
	ASSERT(board->hisPly >= 0 && board->hisPly < CHESS_MAX_GAME_MOVES);
	ASSERT(board->ply >= 0 && board->ply < CHESS_MAX_SEARCH_DEPTH);
	
	board->history[board->hisPly].posKey = board->posKey;
	
	if(move & MFLAGEP) {
        if(side == COLOR_TYPE_WHITE) {
            ClearPiece(to-10,board);
        } else {
            ClearPiece(to+10,board);
        }
    } else if (move & MFLAGCA) {
        switch(to) {
            case C1:
                MovePiece(A1, D1, board);
			break;
            case C8:
                MovePiece(A8, D8, board);
			break;
            case G1:
                MovePiece(H1, F1, board);
			break;
            case G8:
                MovePiece(H8, F8, board);
			break;
            default: ASSERT(BOOL_TYPE_FALSE); break;
        }
    }	
	
	if(board->enPas != NO_SQ) HASH_EP;
    HASH_CA;
	
	board->history[board->hisPly].move = move;
    board->history[board->hisPly].fiftyMove = board->fiftyMove;
    board->history[board->hisPly].enPas = board->enPas;
    board->history[board->hisPly].castlePerm = board->castlePerm;

    board->castlePerm &= CastlePerm[from];
    board->castlePerm &= CastlePerm[to];
    board->enPas = NO_SQ;

	HASH_CA;
	
	int captured = MOVE_GET_CAPTURED(move);
    board->fiftyMove++;
	
	if(captured != EMPTY) {
        ASSERT(PieceValid(captured));
        ClearPiece(to, board);
        board->fiftyMove = 0;
        
        // Track captured piece
        if(captured >= PIECE_TYPE_WHITE_PAWN && captured <= PIECE_TYPE_WHITE_KING) {
            // White piece captured
            if(board->capturedWhiteCount < 16) {
                board->capturedWhite[board->capturedWhiteCount++] = captured;
            }
        } else if(captured >= PIECE_TYPE_BLACK_PAWN && captured <= PIECE_TYPE_BLACK_KING) {
            // Black piece captured
            if(board->capturedBlackCount < 16) {
                board->capturedBlack[board->capturedBlackCount++] = captured;
            }
        }
    }
	
	board->hisPly++;
	board->ply++;
	
	ASSERT(board->hisPly >= 0 && board->hisPly < CHESS_MAX_GAME_MOVES);
	ASSERT(board->ply >= 0 && board->ply < CHESS_MAX_SEARCH_DEPTH);
	
	if(g_piecePawn[board->pieces[from]]) {
        board->fiftyMove = 0;
        if(move & MFLAGPS) {
            if(side==COLOR_TYPE_WHITE) {
                board->enPas=from+10;
                ASSERT(g_ranksBoard[board->enPas]==RANK_TYPE_3);
            } else {
                board->enPas=from-10;
                ASSERT(g_ranksBoard[board->enPas]==RANK_TYPE_6);
            }
            HASH_EP;
        }
    }
	
	MovePiece(from, to, board);
	
	int promotedPiece = MOVE_GET_PROMOTED(move);
    if(promotedPiece != EMPTY)   {
        ASSERT(PieceValid(promotedPiece) && !g_piecePawn[promotedPiece]);
        ClearPiece(to, board);
        AddPiece(to, board, promotedPiece);
    }
	
	if(g_pieceKing[board->pieces[to]]) {
        board->KingSq[board->side] = to;
    }
	
	board->side ^= 1;
    HASH_SIDE;

    ASSERT(Board_Check(board));
	
		
	if(Attack_IsSquareAttacked(board->KingSq[side],board->side,board))  {
        Move_Take(board);
        return BOOL_TYPE_FALSE;
    }
	
	return BOOL_TYPE_TRUE;
	
}

void Move_Take(ChessBoard *board) {
	
	ASSERT(Board_Check(board));
	
	board->hisPly--;
    board->ply--;
	
	ASSERT(board->hisPly >= 0 && board->hisPly < CHESS_MAX_GAME_MOVES);
	ASSERT(board->ply >= 0 && board->ply < CHESS_MAX_SEARCH_DEPTH);
	
    int move = board->history[board->hisPly].move;
    int from = MOVE_GET_FROM_SQUARE(move);
    int to = MOVE_GET_TO_SQUARE(move);	
	
	ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
	
	if(board->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    board->castlePerm = board->history[board->hisPly].castlePerm;
    board->fiftyMove = board->history[board->hisPly].fiftyMove;
    board->enPas = board->history[board->hisPly].enPas;

    if(board->enPas != NO_SQ) HASH_EP;
    HASH_CA;

    board->side ^= 1;
    HASH_SIDE;
	
	if(MFLAGEP & move) {
        if(board->side == COLOR_TYPE_WHITE) {
            AddPiece(to-10, board, PIECE_TYPE_BLACK_PAWN);
        } else {
            AddPiece(to+10, board, PIECE_TYPE_WHITE_PAWN);
        }
    } else if(MFLAGCA & move) {
        switch(to) {
            case C1: MovePiece(D1, A1, board); break;
            case C8: MovePiece(D8, A8, board); break;
            case G1: MovePiece(F1, H1, board); break;
            case G8: MovePiece(F8, H8, board); break;
            default: ASSERT(BOOL_TYPE_FALSE); break;
        }
    }
	
	MovePiece(to, from, board);
	
	if(g_pieceKing[board->pieces[from]]) {
        board->KingSq[board->side] = from;
    }
	
	int captured = MOVE_GET_CAPTURED(move);
    if(captured != EMPTY) {
        ASSERT(PieceValid(captured));
        AddPiece(to, board, captured);
        
        // Remove from captured pieces list
        if(captured >= PIECE_TYPE_WHITE_PAWN && captured <= PIECE_TYPE_WHITE_KING) {
            // White piece was captured, remove it from list
            if(board->capturedWhiteCount > 0) {
                board->capturedWhiteCount--;
            }
        } else if(captured >= PIECE_TYPE_BLACK_PAWN && captured <= PIECE_TYPE_BLACK_KING) {
            // Black piece was captured, remove it from list
            if(board->capturedBlackCount > 0) {
                board->capturedBlackCount--;
            }
        }
    }
	
	if(MOVE_GET_PROMOTED(move) != EMPTY)   {
        ASSERT(PieceValid(MOVE_GET_PROMOTED(move)) && !g_piecePawn[MOVE_GET_PROMOTED(move)]);
        ClearPiece(from, board);
        AddPiece(from, board, (g_pieceCol[MOVE_GET_PROMOTED(move)] == COLOR_TYPE_WHITE ? PIECE_TYPE_WHITE_PAWN : PIECE_TYPE_BLACK_PAWN));
    }
	
    ASSERT(Board_Check(board));

}


void MakeNullMove(ChessBoard *board) {

    ASSERT(Board_Check(board));
    ASSERT(!Attack_IsSquareAttacked(board->KingSq[board->side],board->side^1,board));

    board->ply++;
    board->history[board->hisPly].posKey = board->posKey;

    if(board->enPas != NO_SQ) HASH_EP;

    board->history[board->hisPly].move = NOMOVE;
    board->history[board->hisPly].fiftyMove = board->fiftyMove;
    board->history[board->hisPly].enPas = board->enPas;
    board->history[board->hisPly].castlePerm = board->castlePerm;
    board->enPas = NO_SQ;

    board->side ^= 1;
    board->hisPly++;
    HASH_SIDE;
   
    ASSERT(Board_Check(board));
	ASSERT(board->hisPly >= 0 && board->hisPly < CHESS_MAX_GAME_MOVES);
	ASSERT(board->ply >= 0 && board->ply < CHESS_MAX_SEARCH_DEPTH);

    return;
} // MakeNullMove

void TakeNullMove(ChessBoard *board) {
    ASSERT(Board_Check(board));

    board->hisPly--;
    board->ply--;

    if(board->enPas != NO_SQ) HASH_EP;

    board->castlePerm = board->history[board->hisPly].castlePerm;
    board->fiftyMove = board->history[board->hisPly].fiftyMove;
    board->enPas = board->history[board->hisPly].enPas;

    if(board->enPas != NO_SQ) HASH_EP;
    board->side ^= 1;
    HASH_SIDE;
  
    ASSERT(Board_Check(board));
	ASSERT(board->hisPly >= 0 && board->hisPly < CHESS_MAX_GAME_MOVES);
	ASSERT(board->ply >= 0 && board->ply < CHESS_MAX_SEARCH_DEPTH);
}













