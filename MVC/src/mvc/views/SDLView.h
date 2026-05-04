// SDLView.h — SDL-backed IView (all SDL/types usage stays in .cpp).
#ifndef SOLID_MVC_SDlVIEW_H
#define SOLID_MVC_SDlVIEW_H

#include "../IView.h"
#include "../../ui/sdl/sdl_gui.h"

class SDLView : public IView {
public:
    SDLView();
    ~SDLView() override;

    bool initializeGraphicsSession() override;
    void signalStop() override;
    void notifyModelNewGame() override;
    void showStartupHints() const override;
    void pollInputEvents(std::vector<InputEvent>& events, bool& running) override;

    void updateSelection(int selectedSquare, const std::vector<int>& possibleMoves) override;
    void clearSelection() override;
    void setPromotionPending(bool pending, int fromSq, int toSq) override;
    char promotionChoiceFromClick(int mouseX, int mouseY) override;
    void addMoveToHistory(const char* moveStr) override;
    void trackCapture(int capturedPiece) override;
    void addIncrementForPreviousMover(int previousMover) override;
    bool isTimerActive() const override;
    void startTimer() override;
    void scrollHistory(int direction) override;
    void setGameOverMessage(const char* msg) override;
    void clearGameOver() override;
    bool isGameOver() const override;
    int getGameMode() const override;
    void setGameMode(int mode) override;
    void timerTickThenRender(ChessBoard* board) override;
    void frameDelayMs(int ms) override;

private:
    GUI* activeGui();
    const GUI* activeGui() const;

    static void resetForNewGame(GUI* gui);
    static void handleScrollEvent(GUI* gui, int direction);

    GUI gui_;
    GUI* guiPtr_;
    bool gfxInitialized_;
};

#endif // SOLID_MVC_SDlVIEW_H
