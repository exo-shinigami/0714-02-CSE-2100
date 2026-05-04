// ControllerImpl.cpp — coordinates model + view; no SDL/toolkit dependencies.
#include "ControllerImpl.h"

#include "types_definitions.h"
#include "../../ui/sdl/services/promotion_service.h"
#include "../../ui/sdl/services/engine_move_policy.h"

#include <cstdio>
#include <cstring>

static bool isOwnPiece(ChessBoard* board, int square) {
    if (!board) return false;
    const int piece = board->pieceAt(square);
    if (piece == EMPTY || piece == OFFBOARD) return false;
    const bool isWhiteToMove = (board->getSide() == COLOR_TYPE_WHITE);
    return (isWhiteToMove && piece >= PIECE_TYPE_WHITE_PAWN && piece <= PIECE_TYPE_WHITE_KING)
        || (!isWhiteToMove && piece >= PIECE_TYPE_BLACK_PAWN && piece <= PIECE_TYPE_BLACK_KING);
}

static void buildMoveString(int fromSq, int toSq, char promotion, char* out, size_t outSize) {
    if (promotion != '\0') {
        std::snprintf(out, outSize, "%c%c%c%c%c",
                      fileChar[g_filesBoard[fromSq]],
                      rankChar[g_ranksBoard[fromSq]],
                      fileChar[g_filesBoard[toSq]],
                      rankChar[g_ranksBoard[toSq]],
                      promotion);
        return;
    }

    std::snprintf(out, outSize, "%c%c%c%c",
                  fileChar[g_filesBoard[fromSq]],
                  rankChar[g_ranksBoard[fromSq]],
                  fileChar[g_filesBoard[toSq]],
                  rankChar[g_ranksBoard[toSq]]);
}

static void collectPossibleMoves(ChessBoard* board, int fromSquare, std::vector<int>& out) {
    std::printf("[DEBUG] collectPossibleMoves enter\n"); std::fflush(stdout);
    out.clear();
    if (!board) return;

    MoveList list[1];
    std::printf("[DEBUG] calling moveGenerateAll\n"); std::fflush(stdout);
    moveGenerateAll(board, list);
    std::printf("[DEBUG] after moveGenerateAll\n"); std::fflush(stdout);
    std::printf("[DEBUG] move list size=%d\n", list->size()); std::fflush(stdout);
    for (int i = 0; i < list->size(); ++i) {
        std::printf("[DEBUG] collectPossibleMoves loop i=%d\n", i); std::fflush(stdout);
        const int move = list->at(i).raw();
        std::printf("[DEBUG] move raw=%d\n", move); std::fflush(stdout);
        std::printf("[DEBUG] about to board->makeMove(%d)\n", move); std::fflush(stdout);
        if (board->makeMove(move)) {
            std::printf("[DEBUG] board->makeMove succeeded\n"); std::fflush(stdout);
            const int from = MOVE_GET_FROM_SQUARE(move);
            const int to = MOVE_GET_TO_SQUARE(move);
            board->takeMove();
            if (from == fromSquare) {
                out.push_back(to);
            }
        }
    }
}

static bool updateGameOverIfNeeded(IView* view, ChessBoard* board, const char* label) {
    if (!view || !board) return false;
    if (checkresult(board) != BOOL_TYPE_TRUE) {
        return false;
    }

    char message[256] = {};
    if (board->getFiftyMoveCounter() > 100) {
        std::snprintf(message, sizeof(message), "%s", "DRAW! Fifty move Rule");
    } else if (threeFoldRep(board) >= 2) {
        std::snprintf(message, sizeof(message), "%s", "DRAW! Threefold Repetition");
    } else if (drawMaterial(board) == BOOL_TYPE_TRUE) {
        std::snprintf(message, sizeof(message), "%s", "DRAW! Insufficient Material");
    } else {
        const int inCheck = board->isSquareAttacked(board->getKingSquare(board->getSide()), board->getSide() ^ 1);
        if (inCheck) {
            if (board->getSide() == COLOR_TYPE_WHITE) {
                std::snprintf(message, sizeof(message), "%s", "CHECKMATE! Black Wins!");
            } else {
                std::snprintf(message, sizeof(message), "%s", "CHECKMATE! White Wins!");
            }
        } else {
            std::snprintf(message, sizeof(message), "%s", "DRAW! Stalemate");
        }
    }

    view->setGameOverMessage(message);
    if (label != nullptr) {
        std::printf("*** GAME OVER (%s) ***\n", label);
    }
    return true;
}

ControllerImpl::ControllerImpl(IModel* model, IView* view, SearchInfo* info, const IEvaluator& eval,
                               const IEngineMovePolicy& movePolicy)
    : model_(model),
      view_(view),
      info_(info),
      evaluator_(&eval),
      movePolicy_(&movePolicy),
    running_(false),
    selectedSquare_(NO_SQ),
    promotionFrom_(NO_SQ),
    promotionTo_(NO_SQ),
    promotionPending_(false),
    possibleMoves_() {}

ControllerImpl::~ControllerImpl() {
    running_ = false;
}

void ControllerImpl::start() {
    std::printf("[DEBUG] ControllerImpl::start() begin\n");
    std::fflush(stdout);
    std::printf("[DEBUG] model_ = %p, view_ = %p\n", (void*)model_, (void*)view_);
    std::fflush(stdout);
    if (!model_ || !view_ || !info_) {
        return;
    }
    if (!view_->initializeGraphicsSession()) {
        return;
    }

    running_ = true;

    info_->depth = 6;
    info_->quit = BOOL_TYPE_FALSE;
    info_->stopped = BOOL_TYPE_FALSE;
    info_->gameMode = MODE_TYPE_CONSOLE;

    model_->newGame();
    view_->notifyModelNewGame();
    view_->showStartupHints();
    view_->clearSelection();
    view_->clearGameOver();
    selectedSquare_ = NO_SQ;
    promotionPending_ = false;

    while (running_) {
        std::printf("[DEBUG] ControllerImpl::start() entering loop\n");
    std::fflush(stdout);
    std::vector<InputEvent> events;
        std::printf("[DEBUG] Before view_->pollInputEvents()\n");
        std::fflush(stdout);
        view_->pollInputEvents(events, running_);
        std::printf("[DEBUG] After view_->pollInputEvents()\n");
        std::fflush(stdout);
        if (!running_) {
            break;
        }
        for (const auto& event : events) {
            handleInputEvent(event);
            if (!running_) {
                break;
            }
        }
        if (!running_) {
            break;
        }
        view_->timerTickThenRender(model_->getBoard());
        view_->frameDelayMs(16);
    }
}

void ControllerImpl::stop() {
    running_ = false;
    if (view_) {
        view_->signalStop();
    }
}

void ControllerImpl::handleInputEvent(const InputEvent& event) {
    std::printf("[DEBUG] handleInputEvent enter: action=%d\n", (int)event.action); std::fflush(stdout);
    if (!model_ || !view_) return;

    ChessBoard* board = model_->getBoard();
    if (!board) return;

    switch (event.action) {
        case InputAction::QUIT:
            running_ = false;
            view_->signalStop();
            return;
        case InputAction::NEW_GAME:
            model_->newGame();
            view_->notifyModelNewGame();
            view_->clearGameOver();
            view_->clearSelection();
            selectedSquare_ = NO_SQ;
            promotionPending_ = false;
            return;
        case InputAction::SWITCH_MODE: {
            const int current = view_->getGameMode();
            const int next = (current == GAME_MODE_PVE) ? GAME_MODE_PVP : GAME_MODE_PVE;
            view_->setGameMode(next);
            std::printf("Switched to %s mode\n", next == GAME_MODE_PVE ? "Player vs Engine" : "Player vs Player");
            std::printf("Press 'N' for new game to apply mode change\n");
            return;
        }
        case InputAction::SHOW_HELP:
            std::printf("\n=== CONTROLS ===\n");
            std::printf("N - New Game\n");
            std::printf("M - Switch Mode (PvE/PvP)\n");
            std::printf("H - Show this help\n");
            std::printf("Current mode: %s\n", view_->getGameMode() == GAME_MODE_PVE ? "Player vs Engine" : "Player vs Player");
            std::printf("================\n\n");
            return;
        case InputAction::SCROLL_UP:
            view_->scrollHistory(1);
            return;
        case InputAction::SCROLL_DOWN:
            view_->scrollHistory(-1);
            return;
        case InputAction::SQUARE_CLICKED:
            break;
        case InputAction::NONE:
        default:
            return;
    }

    std::printf("[DEBUG] Checking isGameOver()\n"); std::fflush(stdout);
    if (view_->isGameOver()) {
        return;
    }

    std::printf("[DEBUG] Checking updateGameOverIfNeeded()\n"); std::fflush(stdout);
    if (updateGameOverIfNeeded(view_, board, "before click")) {
        return;
    }

    std::printf("[DEBUG] Handling click block...\n"); std::fflush(stdout);
    const int clickedSquare = event.squareIndex;
    if (clickedSquare == NO_SQ) {
        return;
    }

    std::printf("[DEBUG] Checking promotionPending_\n"); std::fflush(stdout);
    if (promotionPending_) {
        const char choice = view_->promotionChoiceFromClick(event.mouseX, event.mouseY);
        if (choice == '\0') {
            return;
        }

        char moveStr[8];
        buildMoveString(promotionFrom_, promotionTo_, choice, moveStr, sizeof(moveStr));
        const int move = moveParse(moveStr, board);
        if (move == NOMOVE || !model_->applyMoveRaw(move)) {
            view_->clearSelection();
            selectedSquare_ = NO_SQ;
            promotionPending_ = false;
            view_->setPromotionPending(false, NO_SQ, NO_SQ);
            return;
        }

        view_->trackCapture(MOVE_GET_CAPTURED(move));
        view_->addMoveToHistory(moveStr);
        view_->clearSelection();
        selectedSquare_ = NO_SQ;
        promotionPending_ = false;
        view_->setPromotionPending(false, NO_SQ, NO_SQ);

        if (!view_->isTimerActive()) {
            view_->startTimer();
        }
        view_->addIncrementForPreviousMover(board->getSide() ^ 1);

        if (!board->check()) {
            std::printf("✗ Board check failed after move\n");
            return;
        }

        if (updateGameOverIfNeeded(view_, board, "after promotion")) {
            return;
        }

        if (view_->getGameMode() == GAME_MODE_PVE) {
            const int engineMove = movePolicy_->selectMove(*board, *evaluator_);
            if (engineMove != NOMOVE && model_->applyMoveRaw(engineMove)) {
                view_->trackCapture(MOVE_GET_CAPTURED(engineMove));
                view_->addMoveToHistory(prMove(engineMove));
                view_->addIncrementForPreviousMover(board->getSide() ^ 1);
            } else {
                std::printf("✗ Engine couldn't find a move!\n");
            }
            updateGameOverIfNeeded(view_, board, "after engine promotion reply");
        }
        return;
    }

    std::printf("[DEBUG] selectedSquare_ == NO_SQ\n"); std::fflush(stdout);
    if (selectedSquare_ == NO_SQ) {
        std::printf("[DEBUG] Calling isOwnPiece(%p, %d)\n", (void*)board, clickedSquare); std::fflush(stdout);
        if (!isOwnPiece(board, clickedSquare)) {
            return;
        }
        selectedSquare_ = clickedSquare;
        std::printf("[DEBUG] collectPossibleMoves\n"); std::fflush(stdout);
        collectPossibleMoves(board, selectedSquare_, possibleMoves_);
        std::printf("[DEBUG] return collectPossibleMoves\n"); std::fflush(stdout);
        view_->updateSelection(selectedSquare_, possibleMoves_);
        return;
    }

    if (clickedSquare == selectedSquare_) {
        view_->clearSelection();
        selectedSquare_ = NO_SQ;
        return;
    }

    if (PromotionService::isPromotionMove(*board, selectedSquare_, clickedSquare)) {
        promotionPending_ = true;
        promotionFrom_ = selectedSquare_;
        promotionTo_ = clickedSquare;
        view_->setPromotionPending(true, promotionFrom_, promotionTo_);
        return;
    }

    char moveStr[8];
    buildMoveString(selectedSquare_, clickedSquare, '\0', moveStr, sizeof(moveStr));
    const int move = moveParse(moveStr, board);
    if (move == NOMOVE || !model_->applyMoveRaw(move)) {
        view_->clearSelection();
        selectedSquare_ = NO_SQ;
        updateGameOverIfNeeded(view_, board, "after invalid move");
        return;
    }

    view_->trackCapture(MOVE_GET_CAPTURED(move));
    view_->addMoveToHistory(moveStr);
    view_->clearSelection();
    selectedSquare_ = NO_SQ;

    if (!view_->isTimerActive()) {
        view_->startTimer();
    }
    view_->addIncrementForPreviousMover(board->getSide() ^ 1);

    if (!board->check()) {
        std::printf("✗ Board check failed after move\n");
        return;
    }

    if (updateGameOverIfNeeded(view_, board, "after player move")) {
        return;
    }

    if (view_->getGameMode() != GAME_MODE_PVE) {
        return;
    }

    const int engineMove = movePolicy_->selectMove(*board, *evaluator_);
    if (engineMove != NOMOVE && model_->applyMoveRaw(engineMove)) {
        view_->trackCapture(MOVE_GET_CAPTURED(engineMove));
        view_->addMoveToHistory(prMove(engineMove));
        view_->addIncrementForPreviousMover(board->getSide() ^ 1);
    } else {
        std::printf("✗ Engine couldn't find a move!\n");
    }

    updateGameOverIfNeeded(view_, board, "after engine move");
}
