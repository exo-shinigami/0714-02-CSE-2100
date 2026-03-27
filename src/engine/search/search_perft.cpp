/**
 * @file search_perft.c
 * @brief Performance testing (perft) for move generation validation
 * 
 * perft (Performance Test) is a debugging function that counts all
 * leaf nodes at a given depth. It's used to:
 * - Validate move generation correctness
 * - Test make/unmake move functions
 * - Benchmark move generation speed
 * - Compare with known perft values from test positions
 * 
 * The function performs an exhaustive tree search to a specified depth
 * and counts all positions reached, dividing by root move.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "types_definitions.h"
#include "stdio.h"

void PerftRunner::runRecursive(int depth) {
    ASSERT(board_ != nullptr);
    ASSERT(board_->check());

    if (depth == 0) {
        leafNodes_++;
        return;
    }

    MoveList list[1];
    moveGenerateAll(board_, list);

    for (int moveNum = 0; moveNum < list->size(); ++moveNum) {
        if (!board_->makeMove(list->at(moveNum).raw())) {
            continue;
        }
        runRecursive(depth - 1);
        board_->takeMove();
    }
}

long PerftRunner::run(int depth) {
    leafNodes_ = 0;
    runRecursive(depth);
    return leafNodes_;
}

void PerftRunner::test(int depth) {
    ASSERT(board_ != nullptr);
    ASSERT(board_->check());

    board_->print();
    printf("\nStarting Test To Depth:%d\n", depth);
    leafNodes_ = 0;
    int start = miscGetTimeMs();
    MoveList list[1];
    moveGenerateAll(board_, list);
    
    int move;
    for (int moveNum = 0; moveNum < list->size(); ++moveNum) {
        move = list->at(moveNum).raw();
        if (!board_->makeMove(move)) {
            continue;
        }
        long cumnodes = leafNodes_;
        runRecursive(depth - 1);
        board_->takeMove();
        long oldnodes = leafNodes_ - cumnodes;
        printf("move %d : %s : %ld\n", moveNum + 1, prMove(move), oldnodes);
    }

    printf("\nTest Complete : %ld nodes visited in %dms\n", leafNodes_, miscGetTimeMs() - start);
}

void searchPerftTest(int depth, ChessBoard *board) {
    ASSERT(board != nullptr);
    PerftRunner runner(*board);
    runner.test(depth);
}












