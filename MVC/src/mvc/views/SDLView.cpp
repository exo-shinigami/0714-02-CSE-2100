// SDLView.cpp — SDL-backed IView; toolkit and layout types stay here.
#include "SDLView.h"

#include "types_definitions.h"
#include "../../ui/sdl/input/gui_input_handler.h"
#include "../../ui/sdl/services/promotion_service.h"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>

SDLView::SDLView()
    : guiPtr_(nullptr),
      gfxInitialized_(false) {
    GUI zeroGui = {};
    gui_ = zeroGui;
    guiPtr_ = &gui_;
}

SDLView::~SDLView() {
    if (gfxInitialized_) {
        cleanupGUI(&gui_);
        gfxInitialized_ = false;
    }
}

GUI* SDLView::activeGui() {
    return guiPtr_ ? guiPtr_ : &gui_;
}

const GUI* SDLView::activeGui() const {
    return guiPtr_ ? guiPtr_ : &gui_;
}

bool SDLView::initializeGraphicsSession() {
    if (guiPtr_ != &gui_) {
        return false;
    }
    if (gfxInitialized_) {
        return true;
    }
    if (!gUIInit(&gui_)) {
        return false;
    }
    gfxInitialized_ = true;
    return true;
}

void SDLView::signalStop() {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (gui) {
        gui->runtime.isRunning = 0;
    }
}

void SDLView::resetForNewGame(GUI* gui) {
    if (!gui) {
        return;
    }

    gui->selection.selectedSquare = NO_SQ;
    gui->runtime.gameOver = 0;
    std::strcpy(gui->runtime.gameOverMessage, "");

    gui->selection.possibleMovesCount = 0;
    gui->promotion.pending = 0;
    gui->promotion.fromSq = NO_SQ;
    gui->promotion.toSq = NO_SQ;

    gui->moveHistoryTracker.clear();
    gui->moveCount = gui->moveHistoryTracker.getMoveCount();
    gui->historyScrollOffset = gui->moveHistoryTracker.getScrollOffset();
    for (int i = 0; i < MAX_DISPLAY_MOVES; ++i) {
        std::strcpy(gui->moveHistory[i], "");
    }

    resetTimers(gui);

    gui->captures.whiteCount = 0;
    gui->captures.blackCount = 0;
}

void SDLView::handleScrollEvent(GUI* gui, int direction) {
    if (!gui) {
        return;
    }
    if (direction > 0) {
        gui->moveHistoryTracker.scrollUp();
        gui->moveHistoryTracker.scrollUp();
    } else {
        gui->moveHistoryTracker.scrollDown();
        gui->moveHistoryTracker.scrollDown();
    }

    gui->historyScrollOffset = gui->moveHistoryTracker.getScrollOffset();
    gui->moveCount = gui->moveHistoryTracker.getMoveCount();
}

void SDLView::notifyModelNewGame() {
    resetForNewGame(activeGui());
}

void SDLView::showStartupHints() const {
    const GUI* gui = activeGui();
    const char* mode =
        (gui && gui->runtime.gameMode == MODE_PVE) ? "Player vs Engine" : "Player vs Player";
    std::printf("\n=== GAMBIT CHESS ===\n");
    std::printf("Current mode: %s\n", mode);
    std::printf("Controls:\n");
    std::printf("  N - New Game\n");
    std::printf("  M - Switch Mode (PvE/PvP)\n");
    std::printf("  H - Show help\n");
    std::printf("=============================\n\n");
}

void SDLView::pollInputEvents(std::vector<InputEvent>& events, bool& running) {
    std::printf("[DEBUG] SDLView::pollInputEvents begin -> guiPtr_ = %p\n", (void*)guiPtr_); std::fflush(stdout);
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) {
        return;
    }

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        InputEvent evt = gui->inputHandler.processEvent(e);
        if (evt.action != InputAction::NONE) {
            events.push_back(evt);
        }
        if (evt.action == InputAction::QUIT) {
            running = false;
            gui->runtime.isRunning = 0;
        }
    }
}

void SDLView::updateSelection(int selectedSquare, const std::vector<int>& possibleMoves) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->selection.selectedSquare = selectedSquare;
    gui->selection.possibleMovesCount = 0;
    const int maxMoves = 256;
    const int count = static_cast<int>(possibleMoves.size());
    const int limit = (count < maxMoves) ? count : maxMoves;
    for (int i = 0; i < limit; ++i) {
        gui->selection.possibleMoves[i] = possibleMoves[i];
        gui->selection.possibleMovesCount++;
    }
}

void SDLView::clearSelection() {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->selection.selectedSquare = NO_SQ;
    gui->selection.possibleMovesCount = 0;
}

void SDLView::setPromotionPending(bool pending, int fromSq, int toSq) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->promotion.pending = pending ? 1 : 0;
    gui->promotion.fromSq = pending ? fromSq : NO_SQ;
    gui->promotion.toSq = pending ? toSq : NO_SQ;
}

char SDLView::promotionChoiceFromClick(int mouseX, int mouseY) {
    return PromotionService::dialogChoiceFromClick(mouseX, mouseY, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void SDLView::addMoveToHistory(const char* moveStr) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    ::addMoveToHistory(gui, moveStr);
}

void SDLView::trackCapture(int capturedPiece) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    if (capturedPiece == EMPTY) return;
    if (capturedPiece >= PIECE_TYPE_WHITE_PAWN && capturedPiece <= PIECE_TYPE_WHITE_KING) {
        if (gui->captures.whiteCount < 16) {
            gui->captures.white[gui->captures.whiteCount++] = capturedPiece;
        }
        return;
    }
    if (capturedPiece >= PIECE_TYPE_BLACK_PAWN && capturedPiece <= PIECE_TYPE_BLACK_KING) {
        if (gui->captures.blackCount < 16) {
            gui->captures.black[gui->captures.blackCount++] = capturedPiece;
        }
    }
}

void SDLView::addIncrementForPreviousMover(int previousMover) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->gameTimer.addIncrement(previousMover);
    gui->whiteTimeMs = gui->gameTimer.getWhiteTime();
    gui->blackTimeMs = gui->gameTimer.getBlackTime();
    gui->timerActive = gui->gameTimer.isActive() ? 1 : 0;
    gui->timerPaused = gui->gameTimer.isPaused() ? 1 : 0;
    gui->lastMoveTime = miscGetTimeMs();
}

bool SDLView::isTimerActive() const {
    const GUI* gui = activeGui();
    if (!gui) return false;
    return gui->gameTimer.isActive();
}

void SDLView::startTimer() {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->gameTimer.start();
}

void SDLView::scrollHistory(int direction) {
    handleScrollEvent(activeGui(), direction);
}

void SDLView::setGameOverMessage(const char* msg) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->runtime.gameOver = 1;
    if (msg) {
        std::snprintf(gui->runtime.gameOverMessage, sizeof(gui->runtime.gameOverMessage), "%s", msg);
    } else {
        std::snprintf(gui->runtime.gameOverMessage, sizeof(gui->runtime.gameOverMessage), "%s", "");
    }
}

void SDLView::clearGameOver() {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->runtime.gameOver = 0;
    std::strcpy(gui->runtime.gameOverMessage, "");
}

bool SDLView::isGameOver() const {
    const GUI* gui = activeGui();
    if (!gui) return false;
    return gui->runtime.gameOver != 0;
}

int SDLView::getGameMode() const {
    const GUI* gui = activeGui();
    if (!gui) return MODE_PVP;
    return gui->runtime.gameMode;
}

void SDLView::setGameMode(int mode) {
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui) return;
    gui->runtime.gameMode = mode;
}

void SDLView::timerTickThenRender(ChessBoard* board) {
    std::printf("[DEBUG] SDLView::timerTickThenRender begin -> board = %p\n", (void*)board); std::fflush(stdout);
    GUI* gui = activeGui();
    std::printf("[DEBUG] activeGui() returned %p\n", (void*)gui); std::fflush(stdout);
    if (!gui || !gui->window) {
        return;
    }
    std::printf("[DEBUG] Before updateTimer\n"); std::fflush(stdout);
    updateTimer(gui, board);
    std::printf("[DEBUG] After updateTimer\n"); std::fflush(stdout);
    std::printf("[DEBUG] Before gUIRenderBoard\n"); std::fflush(stdout);
    gUIRenderBoard(gui, board);
    std::printf("[DEBUG] After gUIRenderBoard\n"); std::fflush(stdout);
}

void SDLView::frameDelayMs(int ms) {
    if (ms > 0) {
        SDL_Delay(static_cast<Uint32>(ms));
    }
}
