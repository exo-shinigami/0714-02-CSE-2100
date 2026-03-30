/**
 * @file engine_move_policy.cpp
 * @brief Engine move policy implementations
 */

#include "engine_move_policy.h"

#include "../sdl_gui.h"

#include <cstdio>

int GreedyEvalMovePolicy::selectMove(ChessBoard& board, const IEvaluator& evaluator) const {
    MoveList list[1];
    moveGenerateAll(&board, list);

    int bestMove = NOMOVE;
    int bestScore = -999999;

    for (int i = 0; i < list->size(); ++i) {
        const int move = list->at(i).raw();
        if (!board.makeMove(move)) {
            continue;
        }

        const int score = -evaluator.evaluate(board);
        board.takeMove();

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

static void trackCapturedPiece(GUI* gui, int move) {
    const int captured = MOVE_GET_CAPTURED(move);
    if (captured == EMPTY) {
        return;
    }

    if (captured >= PIECE_TYPE_WHITE_PAWN && captured <= PIECE_TYPE_WHITE_KING) {
        if (gui->captures.whiteCount < 16) {
            gui->captures.white[gui->captures.whiteCount++] = captured;
        }
    } else if (captured >= PIECE_TYPE_BLACK_PAWN && captured <= PIECE_TYPE_BLACK_KING) {
        if (gui->captures.blackCount < 16) {
            gui->captures.black[gui->captures.blackCount++] = captured;
        }
    }
}

int EngineMoveService::applyMove(GUI* gui, ChessBoard* board, const IEvaluator& evaluator,
                                 const IEngineMovePolicy& policy) {
    if (gui == nullptr || board == nullptr) {
        return 0;
    }

    const int engineMove = policy.selectMove(*board, evaluator);
    if (engineMove == NOMOVE) {
        std::printf("Engine move policy found no legal move.\n");
        return 0;
    }

    char engineMoveStr[10];
    std::snprintf(engineMoveStr, sizeof(engineMoveStr), "%s", prMove(engineMove));
    addMoveToHistory(gui, engineMoveStr);

    if (!board->makeMove(engineMove)) {
        std::printf("Engine move policy produced invalid move: %s\n", engineMoveStr);
        return 0;
    }

    trackCapturedPiece(gui, engineMove);
    gui->gameTimer.addIncrement(board->getSide() ^ 1);
    gui->lastMoveTime = miscGetTimeMs();
    gui->whiteTimeMs = gui->gameTimer.getWhiteTime();
    gui->blackTimeMs = gui->gameTimer.getBlackTime();

    std::printf("Engine played: %s\n", prMove(engineMove));
    return 1;
}

const IEngineMovePolicy& defaultEngineMovePolicy() {
    static GreedyEvalMovePolicy policy;
    return policy;
}
