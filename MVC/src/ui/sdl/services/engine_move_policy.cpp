/**
 * @file engine_move_policy.cpp
 * @brief Engine move policy implementations
 */

#include "engine_move_policy.h"

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

const IEngineMovePolicy& defaultEngineMovePolicy() {
    static GreedyEvalMovePolicy policy;
    return policy;
}
