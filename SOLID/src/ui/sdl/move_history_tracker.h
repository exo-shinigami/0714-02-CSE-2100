/**
 * @file move_history_tracker.h
 * @brief Move History Tracker - Manages move history display (SRP)
 * 
 * This class follows Single Responsibility Principle by focusing only
 * on tracking and managing move history for display.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef MOVE_HISTORY_TRACKER_H
#define MOVE_HISTORY_TRACKER_H

#include <cstring>

#define MAX_DISPLAY_MOVES 100

/**
 * @class MoveHistoryTracker
 * @brief Responsible only for managing move history
 * 
 * SOLID Principles Applied:
 * - SRP: Only manages move history data
 * - OCP: Can extend with PGN export, analysis, etc.
 */
class MoveHistoryTracker {
private:
    char moves_[MAX_DISPLAY_MOVES][10];
    int moveCount_;
    int scrollOffset_;
    
public:
    MoveHistoryTracker();
    
    // Add a move to history
    void addMove(const char* moveStr);
    
    // Clear all history
    void clear();
    
    // Scroll operations
    void scrollUp();
    void scrollDown();
    void resetScroll();
    
    // Query methods
    int getMoveCount() const { return moveCount_; }
    int getScrollOffset() const { return scrollOffset_; }
    const char* getMove(int index) const;
    
    // Get formatted move for display (e.g., "1. e2e4 e7e5")
    void getFormattedMove(int index, char* buffer, size_t bufferSize) const;
    
    // Export to PGN notation (future enhancement)
    // void exportToPGN(const char* filename) const;
};

#endif // MOVE_HISTORY_TRACKER_H
