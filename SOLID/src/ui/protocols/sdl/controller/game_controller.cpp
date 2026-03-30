/**
 * @file game_controller.cpp
 * @brief GameController implementation
 */

#include "game_controller.h"

#include <cstdio>
#include <cstring>

GameController::GameController(ChessBoard* board, SearchInfo* info, const IEvaluator& eval,
                                                             const IEngineMovePolicy& movePolicy)
    : board_(board),
      searchInfo_(info),
      evaluator_(&eval),
            movePolicy_(&movePolicy),
      gameMode_(GameMode::PLAYER_VS_PLAYER),
      gameState_(GameState::ACTIVE),
            isRunning_(false) {}

GameController::~GameController() {
    if (gui_.window != nullptr || gui_.renderer != nullptr) {
        cleanupGUI(&gui_);
    }
}

void GameController::initializeGame() {
    if (!gUIInit(&gui_)) {
        std::printf("Failed to initialize GUI\n");
        isRunning_ = false;
        return;
    }

    board_->parseFromFEN(CHESS_START_FEN);

    searchInfo_->depth = 6;
    searchInfo_->quit = BOOL_TYPE_FALSE;
    searchInfo_->stopped = BOOL_TYPE_FALSE;
    searchInfo_->gameMode = MODE_TYPE_CONSOLE;

    gameMode_ = (gui_.runtime.gameMode == MODE_PVE) ? GameMode::PLAYER_VS_ENGINE : GameMode::PLAYER_VS_PLAYER;
    gameState_ = GameState::ACTIVE;
    isRunning_ = true;

    std::printf("\n=== GAMBIT CHESS ===\n");
    std::printf("Current mode: %s\n", gui_.runtime.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
    std::printf("Controls:\n");
    std::printf("  N - New Game\n");
    std::printf("  M - Switch Mode (PvE/PvP)\n");
    std::printf("  H - Show help\n");
    std::printf("=============================\n\n");
}

void GameController::handleScrollEvent(int direction) {
    if (direction > 0) {
        gui_.moveHistoryTracker.scrollUp();
        gui_.moveHistoryTracker.scrollUp();
    } else {
        gui_.moveHistoryTracker.scrollDown();
        gui_.moveHistoryTracker.scrollDown();
    }

    gui_.historyScrollOffset = gui_.moveHistoryTracker.getScrollOffset();
    gui_.moveCount = gui_.moveHistoryTracker.getMoveCount();
}

void GameController::processSquareClick(int square) {
    if (square == NO_SQ) {
        return;
    }
    int x = 0;
    int y = 0;
    inputHandler_.getSquareCoords(square, &x, &y);
    gUIHandleMouseClick(&gui_, board_, searchInfo_, x + 1, y + 1, *evaluator_, *movePolicy_);
}

void GameController::resetForNewGame() {
    board_->parseFromFEN(CHESS_START_FEN);

    gui_.selection.selectedSquare = NO_SQ;
    gui_.runtime.gameOver = 0;
    std::strcpy(gui_.runtime.gameOverMessage, "");

    gui_.selection.possibleMovesCount = 0;
    gui_.promotion.pending = 0;
    gui_.promotion.fromSq = NO_SQ;
    gui_.promotion.toSq = NO_SQ;

    gui_.moveHistoryTracker.clear();
    gui_.moveCount = gui_.moveHistoryTracker.getMoveCount();
    gui_.historyScrollOffset = gui_.moveHistoryTracker.getScrollOffset();
    for (int i = 0; i < MAX_DISPLAY_MOVES; ++i) {
        std::strcpy(gui_.moveHistory[i], "");
    }

    resetTimers(&gui_);

    gui_.captures.whiteCount = 0;
    gui_.captures.blackCount = 0;

    gameState_ = GameState::ACTIVE;
    std::printf("New game started in %s mode\n", gui_.runtime.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
}

void GameController::printHelp() const {
    std::printf("\n=== CONTROLS ===\n");
    std::printf("N - New Game\n");
    std::printf("M - Switch Mode (PvE/PvP)\n");
    std::printf("H - Show this help\n");
    std::printf("Current mode: %s\n", gui_.runtime.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
    std::printf("================\n\n");
}

void GameController::applyInputAction(const InputEvent& inputEvent) {
    switch (inputEvent.action) {
        case InputAction::QUIT:
            quit();
            break;
        case InputAction::SQUARE_CLICKED:
            if (gui_.promotion.pending) {
                gUIHandleMouseClick(&gui_, board_, searchInfo_, inputEvent.mouseX, inputEvent.mouseY,
                                    *evaluator_, *movePolicy_);
            } else {
                processSquareClick(inputEvent.squareIndex);
            }
            break;
        case InputAction::NEW_GAME:
            resetForNewGame();
            break;
        case InputAction::SWITCH_MODE:
            gui_.runtime.gameMode = (gui_.runtime.gameMode == MODE_PVE) ? MODE_PVP : MODE_PVE;
            gameMode_ = (gui_.runtime.gameMode == MODE_PVE) ? GameMode::PLAYER_VS_ENGINE : GameMode::PLAYER_VS_PLAYER;
            std::printf("Switched to %s mode\n", gui_.runtime.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
            std::printf("Press 'N' for new game to apply mode change\n");
            break;
        case InputAction::SHOW_HELP:
            printHelp();
            break;
        case InputAction::SCROLL_UP:
            handleScrollEvent(1);
            break;
        case InputAction::SCROLL_DOWN:
            handleScrollEvent(-1);
            break;
        case InputAction::NONE:
            break;
    }
}

void GameController::run() {
    initializeGame();
    if (!isRunning_) {
        return;
    }

    SDL_Event e;
    while (isRunning_) {
        while (SDL_PollEvent(&e) != 0) {
            handleEvent(e);
        }

        updateTimer(&gui_, board_);

        if (gui_.runtime.gameOver && gameState_ == GameState::ACTIVE) {
            gameState_ = GameState::CHECKMATE;
        }

        render();
        SDL_Delay(16);
    }
}

void GameController::newGame() {
    resetForNewGame();
}

void GameController::setGameMode(GameMode mode) {
    gameMode_ = mode;
    gui_.runtime.gameMode = (mode == GameMode::PLAYER_VS_ENGINE) ? MODE_PVE : MODE_PVP;
}

void GameController::quit() {
    isRunning_ = false;
    gui_.runtime.isRunning = 0;
}

void GameController::handleEvent(const SDL_Event& event) {
    if (gui_.promotion.pending && event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        gUIHandleMouseClick(&gui_, board_, searchInfo_, event.button.x, event.button.y,
                            *evaluator_, *movePolicy_);
        return;
    }

    if (event.type == SDL_MOUSEWHEEL) {
        handleScrollEvent(event.wheel.y);
        return;
    }

    const InputEvent inputEvent = inputHandler_.processEvent(event);
    applyInputAction(inputEvent);
}

void GameController::render() {
    gUIRenderBoard(&gui_, board_);
}
