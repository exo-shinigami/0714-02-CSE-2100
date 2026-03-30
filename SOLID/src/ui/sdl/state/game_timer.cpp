/**
 * @file game_timer.cpp
 * @brief Implementation of GameTimer class
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#include "game_timer.h"
#include "types_definitions.h"
#include <cstdio>

GameTimer::GameTimer(int initialTimeMs, int incrementMs)
    : whiteTimeMs_(initialTimeMs),
      blackTimeMs_(initialTimeMs),
      incrementMs_(incrementMs),
      lastMoveTime_(0),
      isActive_(false),
      isPaused_(false)
{
}

void GameTimer::start() {
    isActive_ = true;
    isPaused_ = false;
    lastMoveTime_ = miscGetTimeMs();
}

void GameTimer::pause() {
    if (isActive_) {
        isPaused_ = true;
    }
}

void GameTimer::resume() {
    if (isActive_ && isPaused_) {
        isPaused_ = false;
        lastMoveTime_ = miscGetTimeMs();
    }
}

void GameTimer::reset(int initialTimeMs, int incrementMs) {
    whiteTimeMs_ = initialTimeMs;
    blackTimeMs_ = initialTimeMs;
    incrementMs_ = incrementMs;
    lastMoveTime_ = 0;
    isActive_ = false;
    isPaused_ = false;
}

void GameTimer::update(int currentSide) {
    if (!isActive_ || isPaused_) {
        return;
    }
    
    int currentTime = miscGetTimeMs();
    int elapsed = currentTime - lastMoveTime_;
    lastMoveTime_ = currentTime;
    
    // Deduct time from current player
    if (currentSide == COLOR_TYPE_WHITE) {
        whiteTimeMs_ -= elapsed;
        if (whiteTimeMs_ < 0) whiteTimeMs_ = 0;
    } else {
        blackTimeMs_ -= elapsed;
        if (blackTimeMs_ < 0) blackTimeMs_ = 0;
    }
}

void GameTimer::addIncrement(int side) {
    if (side == COLOR_TYPE_WHITE) {
        whiteTimeMs_ += incrementMs_;
    } else {
        blackTimeMs_ += incrementMs_;
    }
}

void GameTimer::formatTime(int timeMs, char* buffer, size_t bufferSize) const {
    int totalSeconds = timeMs / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    
    snprintf(buffer, bufferSize, "%02d:%02d", minutes, seconds);
}
