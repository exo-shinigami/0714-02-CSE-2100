/**
 * @file move_history_tracker.cpp
 * @brief Implementation of MoveHistoryTracker class
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#include "move_history_tracker.h"
#include <cstdio>

MoveHistoryTracker::MoveHistoryTracker()
    : moveCount_(0), scrollOffset_(0)
{
    // Initialize all move strings to empty
    for (int i = 0; i < MAX_DISPLAY_MOVES; i++) {
        moves_[i][0] = '\0';
    }
}

void MoveHistoryTracker::addMove(const char* moveStr) {
    if (moveCount_ < MAX_DISPLAY_MOVES && moveStr != nullptr) {
        strncpy(moves_[moveCount_], moveStr, 9);
        moves_[moveCount_][9] = '\0'; // Ensure null termination
        moveCount_++;
    }
}

void MoveHistoryTracker::clear() {
    moveCount_ = 0;
    scrollOffset_ = 0;
    
    for (int i = 0; i < MAX_DISPLAY_MOVES; i++) {
        moves_[i][0] = '\0';
    }
}

void MoveHistoryTracker::scrollUp() {
    if (scrollOffset_ > 0) {
        scrollOffset_--;
    }
}

void MoveHistoryTracker::scrollDown() {
    const int maxScroll = moveCount_ - 10; // Assuming ~10 visible moves
    if (scrollOffset_ < maxScroll && maxScroll > 0) {
        scrollOffset_++;
    }
}

void MoveHistoryTracker::resetScroll() {
    scrollOffset_ = 0;
}

const char* MoveHistoryTracker::getMove(int index) const {
    if (index >= 0 && index < moveCount_) {
        return moves_[index];
    }
    return nullptr;
}

void MoveHistoryTracker::getFormattedMove(int index, char* buffer, size_t bufferSize) const {
    if (index >= 0 && index < moveCount_) {
        // Format: "1. e2e4 e7e5" for move pairs
        if (index % 2 == 0) {
            int moveNum = (index / 2) + 1;
            if (index + 1 < moveCount_) {
                snprintf(buffer, bufferSize, "%d. %s %s", 
                        moveNum, moves_[index], moves_[index + 1]);
            } else {
                snprintf(buffer, bufferSize, "%d. %s", 
                        moveNum, moves_[index]);
            }
        } else {
            buffer[0] = '\0'; // Odd indices are handled with even indices
        }
    } else {
        buffer[0] = '\0';
    }
}
