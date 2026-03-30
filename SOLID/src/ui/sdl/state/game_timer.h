/**
 * @file game_timer.h
 * @brief Game Timer - Manages chess clock functionality (SRP)
 * 
 * This class follows Single Responsibility Principle by focusing only
 * on timer management. Separate from rendering and input.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef GAME_TIMER_H
#define GAME_TIMER_H

#include <cstddef>

/**
 * @class GameTimer
 * @brief Responsible only for managing chess clock timers
 * 
 * SOLID Principles Applied:
 * - SRP: Only manages time tracking
 * - OCP: Can extend with different time control modes
 */
class GameTimer {
private:
    int whiteTimeMs_;
    int blackTimeMs_;
    int incrementMs_;
    int lastMoveTime_;
    bool isActive_;
    bool isPaused_;
    
public:
    // Constructor with default time control
    GameTimer(int initialTimeMs = 600000, int incrementMs = 0);
    
    // Timer control
    void start();
    void pause();
    void resume();
    void reset(int initialTimeMs = 600000, int incrementMs = 0);
    
    // Update timer based on current side
    void update(int currentSide);
    
    // Add increment after move
    void addIncrement(int side);
    
    // Query methods
    int getWhiteTime() const { return whiteTimeMs_; }
    int getBlackTime() const { return blackTimeMs_; }
    int getIncrement() const { return incrementMs_; }
    bool isActive() const { return isActive_; }
    bool isPaused() const { return isPaused_; }
    
    // Check for time forfeit
    bool hasWhiteTimedOut() const { return whiteTimeMs_ <= 0; }
    bool hasBlackTimedOut() const { return blackTimeMs_ <= 0; }
    
    // Format time for display
    void formatTime(int timeMs, char* buffer, size_t bufferSize) const;
};

#endif // GAME_TIMER_H
