/**
 * @file game_rules.cpp
 * @brief Core game state helper rules (draws, repetition, game end)
 *
 * This module contains board-state rule checks that are reused by protocol
 * frontends. Keeping these checks in core avoids mixing chess rules with
 * protocol parsing/transport logic.
 */

#include "stdio.h"
#include "types_definitions.h"

int ChessBoard::hasAnyLegalMove() {
    MoveList list[1];
    moveGenerateAll(this, list);

    int moveNum = 0;
    for (moveNum = 0; moveNum < list->size(); ++moveNum) {
        if (!makeMove(list->at(moveNum).raw())) {
            continue;
        }
        takeMove();
        return BOOL_TYPE_TRUE;
    }

    return BOOL_TYPE_FALSE;
}

int GameRulesService::repetitionCount(const IBoardQuery& board) {
    ASSERT(board.isValid());

    int i = 0;
    int r = 0;
    for (i = 0; i < board.getHistoryPly(); ++i) {
        if (board.getHistoryPositionKey(i) == board.getPositionKey()) {
            r++;
        }
    }
    return r;
}

int GameRulesService::isDrawByMaterial(const IBoardQuery& board) {
    ASSERT(board.isValid());

    if (board.getPieceCount(PIECE_TYPE_WHITE_PAWN) || board.getPieceCount(PIECE_TYPE_BLACK_PAWN)) return BOOL_TYPE_FALSE;
    if (board.getPieceCount(PIECE_TYPE_WHITE_QUEEN) || board.getPieceCount(PIECE_TYPE_BLACK_QUEEN) || board.getPieceCount(PIECE_TYPE_WHITE_ROOK) || board.getPieceCount(PIECE_TYPE_BLACK_ROOK)) return BOOL_TYPE_FALSE;
    if (board.getPieceCount(PIECE_TYPE_WHITE_BISHOP) > 1 || board.getPieceCount(PIECE_TYPE_BLACK_BISHOP) > 1) return BOOL_TYPE_FALSE;
    if (board.getPieceCount(PIECE_TYPE_WHITE_KNIGHT) > 1 || board.getPieceCount(PIECE_TYPE_BLACK_KNIGHT) > 1) return BOOL_TYPE_FALSE;
    if (board.getPieceCount(PIECE_TYPE_WHITE_KNIGHT) && board.getPieceCount(PIECE_TYPE_WHITE_BISHOP)) return BOOL_TYPE_FALSE;
    if (board.getPieceCount(PIECE_TYPE_BLACK_KNIGHT) && board.getPieceCount(PIECE_TYPE_BLACK_BISHOP)) return BOOL_TYPE_FALSE;

    return BOOL_TYPE_TRUE;
}

int GameRulesService::evaluateTerminalState(IBoardQuery& queryBoard, IBoardModifier& mutableBoard) {
    ASSERT(queryBoard.isValid());

    if (queryBoard.getFiftyMoveCounter() > 100) {
        printf("1/2-1/2 {fifty move rule (claimed by Gambit)}\\n");
        return BOOL_TYPE_TRUE;
    }

    if (repetitionCount(queryBoard) >= 2) {
        printf("1/2-1/2 {3-fold repetition (claimed by Gambit)}\\n");
        return BOOL_TYPE_TRUE;
    }

    if (isDrawByMaterial(queryBoard) == BOOL_TYPE_TRUE) {
        printf("1/2-1/2 {insufficient material (claimed by Gambit)}\\n");
        return BOOL_TYPE_TRUE;
    }

    if (mutableBoard.hasAnyLegalMove() == BOOL_TYPE_TRUE) return BOOL_TYPE_FALSE;

    int inCheck = queryBoard.isSquareAttacked(queryBoard.getKingSquare(queryBoard.getSide()), queryBoard.getSide() ^ 1);

    if (inCheck == BOOL_TYPE_TRUE) {
        if (queryBoard.getSide() == COLOR_TYPE_WHITE) {
            printf("0-1 {black mates (claimed by Gambit)}\\n");
            return BOOL_TYPE_TRUE;
        }
        printf("1-0 {white mates (claimed by Gambit)}\\n");
        return BOOL_TYPE_TRUE;
    }

    printf("\\n1/2-1/2 {stalemate (claimed by Gambit)}\\n");
    return BOOL_TYPE_TRUE;
}

int threeFoldRep(const IBoardQuery *board) {
    ASSERT(board != nullptr);
    return GameRulesService::repetitionCount(*board);
}

int drawMaterial(const IBoardQuery *board) {
    ASSERT(board != nullptr);
    return GameRulesService::isDrawByMaterial(*board);
}

int checkresult(ChessBoard *board) {
    ASSERT(board != nullptr);
    return GameRulesService::evaluateTerminalState(*board, *board);
}
