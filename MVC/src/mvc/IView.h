// IView.h — presentation boundary (no SDL or other toolkit types here).
#ifndef SOLID_MVC_IVIEW_H
#define SOLID_MVC_IVIEW_H

#include <vector>
#include "InputEvent.h"

class ChessBoard;

class IView {
public:
    virtual ~IView() = default;

    virtual bool initializeGraphicsSession() = 0;
    virtual void signalStop() = 0;

    virtual void notifyModelNewGame() = 0;
    virtual void showStartupHints() const = 0;

    /// Drain pending input for one frame iteration (implementations may poll many events).
    virtual void pollInputEvents(std::vector<InputEvent>& events, bool& running) = 0;

    // View-facing state updates (render uses internal GUI state).
    virtual void updateSelection(int selectedSquare, const std::vector<int>& possibleMoves) = 0;
    virtual void clearSelection() = 0;
    virtual void setPromotionPending(bool pending, int fromSq, int toSq) = 0;
    virtual char promotionChoiceFromClick(int mouseX, int mouseY) = 0;
    virtual void addMoveToHistory(const char* moveStr) = 0;
    virtual void trackCapture(int capturedPiece) = 0;
    virtual void addIncrementForPreviousMover(int previousMover) = 0;
    virtual bool isTimerActive() const = 0;
    virtual void startTimer() = 0;
    virtual void scrollHistory(int direction) = 0;
    virtual void setGameOverMessage(const char* msg) = 0;
    virtual void clearGameOver() = 0;
    virtual bool isGameOver() const = 0;
    virtual int getGameMode() const = 0;
    virtual void setGameMode(int mode) = 0;

    virtual void timerTickThenRender(ChessBoard* board) = 0;
    virtual void frameDelayMs(int ms) = 0;
};

#endif // SOLID_MVC_IVIEW_H
