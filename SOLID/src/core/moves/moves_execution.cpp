/**
 * @file moves_execution.c
 * @brief move execution and retraction functions
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

#define HASH_PCE(piece,squareIndex) (board->xorPositionKey(g_pieceKeys[(piece)][(squareIndex)]))
#define HASH_CA (board->xorPositionKey(g_castleKeys[(board->getCastlePermission())]))
#define HASH_SIDE (board->xorPositionKey(g_sideKey))
#define HASH_EP (board->xorPositionKey(g_pieceKeys[EMPTY][(board->getEnPassantSquare())]))

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

static void clearPiece(const int squareIndex, ChessBoard *board) {

	ASSERT(sqOnBoard(squareIndex));
	ASSERT(board->check());
	
    int piece = board->pieceAt(squareIndex);
	
    ASSERT(pieceValid(piece));
	
	int color = g_pieceCol[piece];
	int index = 0;
	int t_pceNum = -1;
	
	ASSERT(sideValid(color));
	
    HASH_PCE(piece,squareIndex);
	
	board->pieceAt(squareIndex) = EMPTY;
    board->materialAt(color) -= g_pieceVal[piece];
	
	if(pieceBig[piece]) {
			board->bigPieceCountAt(color)--;
		if(pieceMaj[piece]) {
			board->majorPieceCountAt(color)--;
		} else {
			board->minorPieceCountAt(color)--;
		}
	} else {
		BITBOARD_CLEAR_BIT(board->pawnsAt(color),SQUARE_120_TO_64(squareIndex));
		BITBOARD_CLEAR_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(squareIndex));
	}
	
	for(index = 0; index < board->pieceCountAt(piece); ++index) {
		if(board->pieceListAt(piece, index) == squareIndex) {
			t_pceNum = index;
			break;
		}
	}
	
	ASSERT(t_pceNum != -1);
	ASSERT(t_pceNum>=0&&t_pceNum<10);
	
	board->pieceCountAt(piece)--;		
	
	board->pieceListAt(piece, t_pceNum) = board->pieceListAt(piece, board->pieceCountAt(piece));
  
}


static void addPiece(const int squareIndex, ChessBoard *board, const int piece) {

    ASSERT(pieceValid(piece));
    ASSERT(sqOnBoard(squareIndex));
	
	int color = g_pieceCol[piece];
	ASSERT(sideValid(color));

    HASH_PCE(piece,squareIndex);
	
	board->pieceAt(squareIndex) = piece;

    if(pieceBig[piece]) {
			board->bigPieceCountAt(color)++;
		if(pieceMaj[piece]) {
			board->majorPieceCountAt(color)++;
		} else {
			board->minorPieceCountAt(color)++;
		}
	} else {
		BITBOARD_SET_BIT(board->pawnsAt(color),SQUARE_120_TO_64(squareIndex));
		BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(squareIndex));
	}
	
	board->materialAt(color) += g_pieceVal[piece];
	board->pieceListAt(piece, board->pieceCountAt(piece)++) = squareIndex;
	
}

static void movePiece(const int from, const int to, ChessBoard *board) {

    ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
	
	int index = 0;
	int piece = board->pieceAt(from);	
	int color = g_pieceCol[piece];
	ASSERT(sideValid(color));
    ASSERT(pieceValid(piece));
	
#ifdef DEBUG
	int t_PieceNum = BOOL_TYPE_FALSE;
#endif

	HASH_PCE(piece,from);
	board->pieceAt(from) = EMPTY;
	
	HASH_PCE(piece,to);
	board->pieceAt(to) = piece;
	
	if(!pieceBig[piece]) {
		BITBOARD_CLEAR_BIT(board->pawnsAt(color),SQUARE_120_TO_64(from));
		BITBOARD_CLEAR_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(from));
		BITBOARD_SET_BIT(board->pawnsAt(color),SQUARE_120_TO_64(to));
		BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(to));		
	}    
	
	for(index = 0; index < board->pieceCountAt(piece); ++index) {
		if(board->pieceListAt(piece, index) == from) {
			board->pieceListAt(piece, index) = to;
#ifdef DEBUG
			t_PieceNum = BOOL_TYPE_TRUE;
#endif
			break;
		}
	}
	ASSERT(t_PieceNum);
}

int ChessBoard::makeMove(int move) {
	ChessBoard *board = this;

	ASSERT(board->check());
	
	int from = MOVE_GET_FROM_SQUARE(move);
    int to = MOVE_GET_TO_SQUARE(move);
    int side = board->getSide();
	
	ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
    ASSERT(sideValid(side));
    ASSERT(pieceValid(board->pieceAt(from)));
    ASSERT(board->getHistoryPly() >= 0 && board->getHistoryPly() < CHESS_MAX_GAME_MOVES);
    ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);
	
    board->history[board->getHistoryPly()].posKey = board->getPositionKey();
	
	if(move & MFLAGEP) {
        if(side == COLOR_TYPE_WHITE) {
            clearPiece(to-10,board);
        } else {
            clearPiece(to+10,board);
        }
    } else if (move & MFLAGCA) {
        switch(to) {
            case C1:
                movePiece(A1, D1, board);
			break;
            case C8:
                movePiece(A8, D8, board);
			break;
            case G1:
                movePiece(H1, F1, board);
			break;
            case G8:
                movePiece(H8, F8, board);
			break;
            default: ASSERT(BOOL_TYPE_FALSE); break;
        }
    }	
	
    if(board->getEnPassantSquare() != NO_SQ) HASH_EP;
    HASH_CA;
	
    board->history[board->getHistoryPly()].move = move;
    board->history[board->getHistoryPly()].fiftyMove = board->getFiftyMoveCounter();
    board->history[board->getHistoryPly()].enPas = board->getEnPassantSquare();
    board->history[board->getHistoryPly()].castlePerm = board->getCastlePermission();

    board->andCastlePermission(CastlePerm[from]);
    board->andCastlePermission(CastlePerm[to]);
    board->setEnPassantSquare(NO_SQ);

	HASH_CA;
	
	int captured = MOVE_GET_CAPTURED(move);
    board->incrementFiftyMoveCounter();
	
	if(captured != EMPTY) {
        ASSERT(pieceValid(captured));
        clearPiece(to, board);
        board->setFiftyMoveCounter(0);
    }
	
    board->incrementHistoryPly();
    board->incrementPly();
	
    ASSERT(board->getHistoryPly() >= 0 && board->getHistoryPly() < CHESS_MAX_GAME_MOVES);
    ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);
	
	if(g_piecePawn[board->pieceAt(from)]) {
        board->setFiftyMoveCounter(0);
        if(move & MFLAGPS) {
            if(side==COLOR_TYPE_WHITE) {
                board->setEnPassantSquare(from+10);
                ASSERT(g_ranksBoard[board->getEnPassantSquare()]==RANK_TYPE_3);
            } else {
                board->setEnPassantSquare(from-10);
                ASSERT(g_ranksBoard[board->getEnPassantSquare()]==RANK_TYPE_6);
            }
            HASH_EP;
        }
    }
	
	movePiece(from, to, board);
	
	int promotedPiece = MOVE_GET_PROMOTED(move);
    if(promotedPiece != EMPTY)   {
        ASSERT(pieceValid(promotedPiece) && !g_piecePawn[promotedPiece]);
        clearPiece(to, board);
        addPiece(to, board, promotedPiece);
    }
	
	if(g_pieceKing[board->pieceAt(to)]) {
        board->setKingSquare(board->getSide(), to);
    }
	
	board->toggleSide();
    HASH_SIDE;

    ASSERT(board->check());
	
		
	if(board->isSquareAttacked(board->getKingSquare(side), board->getSide()))  {
        board->takeMove();
        return BOOL_TYPE_FALSE;
    }
	
	return BOOL_TYPE_TRUE;
	
}

void ChessBoard::takeMove() {
	ChessBoard *board = this;

	ASSERT(board->check());
	
    board->decrementHistoryPly();
    board->decrementPly();
	
    ASSERT(board->getHistoryPly() >= 0 && board->getHistoryPly() < CHESS_MAX_GAME_MOVES);
    ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);
	
    int move = board->history[board->getHistoryPly()].move;
    int from = MOVE_GET_FROM_SQUARE(move);
    int to = MOVE_GET_TO_SQUARE(move);	
	
	ASSERT(sqOnBoard(from));
    ASSERT(sqOnBoard(to));
	
	if(board->getEnPassantSquare() != NO_SQ) HASH_EP;
    HASH_CA;

    board->setCastlePermission(board->history[board->getHistoryPly()].castlePerm);
    board->setFiftyMoveCounter(board->history[board->getHistoryPly()].fiftyMove);
    board->setEnPassantSquare(board->history[board->getHistoryPly()].enPas);

	if(board->getEnPassantSquare() != NO_SQ) HASH_EP;
    HASH_CA;

    board->toggleSide();
    HASH_SIDE;
	
	if(MFLAGEP & move) {
		if(board->getSide() == COLOR_TYPE_WHITE) {
            addPiece(to-10, board, PIECE_TYPE_BLACK_PAWN);
        } else {
            addPiece(to+10, board, PIECE_TYPE_WHITE_PAWN);
        }
    } else if(MFLAGCA & move) {
        switch(to) {
            case C1: movePiece(D1, A1, board); break;
            case C8: movePiece(D8, A8, board); break;
            case G1: movePiece(F1, H1, board); break;
            case G8: movePiece(F8, H8, board); break;
            default: ASSERT(BOOL_TYPE_FALSE); break;
        }
    }
	
	movePiece(to, from, board);
	
	if(g_pieceKing[board->pieceAt(from)]) {
    board->setKingSquare(board->getSide(), from);
    }
	
	int captured = MOVE_GET_CAPTURED(move);
    if(captured != EMPTY) {
        ASSERT(pieceValid(captured));
        addPiece(to, board, captured);
    }
	
	if(MOVE_GET_PROMOTED(move) != EMPTY)   {
        ASSERT(pieceValid(MOVE_GET_PROMOTED(move)) && !g_piecePawn[MOVE_GET_PROMOTED(move)]);
        clearPiece(from, board);
        addPiece(from, board, (g_pieceCol[MOVE_GET_PROMOTED(move)] == COLOR_TYPE_WHITE ? PIECE_TYPE_WHITE_PAWN : PIECE_TYPE_BLACK_PAWN));
    }
	
    ASSERT(board->check());

}


void ChessBoard::makeNullMove() {
	ChessBoard *board = this;

    ASSERT(board->check());
    ASSERT(!board->isSquareAttacked(board->getKingSquare(board->getSide()),board->getSide()^1));

    board->incrementPly();
    board->history[board->getHistoryPly()].posKey = board->getPositionKey();

	if(board->getEnPassantSquare() != NO_SQ) HASH_EP;

    board->history[board->getHistoryPly()].move = NOMOVE;
    board->history[board->getHistoryPly()].fiftyMove = board->getFiftyMoveCounter();
    board->history[board->getHistoryPly()].enPas = board->getEnPassantSquare();
    board->history[board->getHistoryPly()].castlePerm = board->getCastlePermission();
    board->setEnPassantSquare(NO_SQ);

    board->toggleSide();
    board->incrementHistoryPly();
    HASH_SIDE;
   
    ASSERT(board->check());
    ASSERT(board->getHistoryPly() >= 0 && board->getHistoryPly() < CHESS_MAX_GAME_MOVES);
    ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);

    return;
} // makeNullMove

void ChessBoard::takeNullMove() {
	ChessBoard *board = this;
    ASSERT(board->check());

    board->decrementHistoryPly();
    board->decrementPly();

	if(board->getEnPassantSquare() != NO_SQ) HASH_EP;

    board->setCastlePermission(board->history[board->getHistoryPly()].castlePerm);
    board->setFiftyMoveCounter(board->history[board->getHistoryPly()].fiftyMove);
    board->setEnPassantSquare(board->history[board->getHistoryPly()].enPas);

	if(board->getEnPassantSquare() != NO_SQ) HASH_EP;
    board->toggleSide();
    HASH_SIDE;
  
    ASSERT(board->check());
    ASSERT(board->getHistoryPly() >= 0 && board->getHistoryPly() < CHESS_MAX_GAME_MOVES);
    ASSERT(board->getPly() >= 0 && board->getPly() < CHESS_MAX_SEARCH_DEPTH);
}













