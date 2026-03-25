/**
 * @file game_controller.h
 * @brief Game Controller - Coordinates game flow and components (SRP)
 * 
 * This class follows Single Responsibility Principle by focusing only
 * on coordinating the game flow between components.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "types_definitions.h"
#include "gui_renderer.h"
#include "gui_input_handler.h"
#include "game_timer.h"
#include "move_history_tracker.h"
#include <SDL2/SDL.h>

/**
 * @enum GameMode
 * @brief Different game modes available
 */
enum class GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_VS_ENGINE,
    ENGINE_VS_ENGINE
};

/**
 * @enum GameState
 * @brief Current state of the game
 */
enum class GameState {
    ACTIVE,
    CHECKMATE,
    STALEMATE,
    DRAW_FIFTY_MOVE,
    DRAW_REPETITION,
    DRAW_MATERIAL,
    TIME_FORFEIT
};

/**
 * @class GameController
 * @brief Coordinates game flow and component interaction
 * 
 * SOLID Principles Applied:
 * - SRP: Only coordinates game flow, doesn't render or handle input directly
 * - DIP: Depends on abstractions (IEvaluator) not concrete classes
 * - OCP: Can add new game modes without modifying existing code
 * 
 * This is the "Facade" pattern - provides simple interface to complex subsystem
 */
class GameController {
private:
    // Game state
    ChessBoard* board_;
    SearchInfo* searchInfo_;
    const IEvaluator* evaluator_;
    
    // Components (following SRP)
    GUIRenderer* renderer_;
    GUIInputHandler* inputHandler_;
    GameTimer* timer_;
    MoveHistoryTracker* history_;
    
    // GUI internals
    SDL_Window* window_;
    SDL_Renderer* sdlRenderer_;
    
    // Game state
    GameMode gameMode_;
    GameState gameState_;
    int selectedSquare_;
    int possibleMoves_[256];
    int possibleMovesCount_;
    bool isRunning_;
    
    // Captured pieces tracking
    int capturedWhite_[16];
    int capturedBlack_[16];
    int capturedWhiteCount_;
    int capturedBlackCount_;
    
    // Promotion handling
    bool promotionPending_;
    int promotionFromSq_;
    int promotionToSq_;
    
    // Private helper methods
    void initializeGame();
    void processSquareClick(int square);
    void executeMove(int move);
    void updateGameState();
    void triggerAIMove();
    bool isPlayerTurn() const;
    void generatePossibleMoves();
    void clearPossibleMoves();
    void trackCapturedPiece(int capturedPiece);
    
public:
    GameController(ChessBoard* board, SearchInfo* info, const IEvaluator& eval);
    ~GameController();
    
    // Non-copyable
    GameController(const GameController&) = delete;
    GameController& operator=(const GameController&) = delete;
    
    // Main game loop
    void run();
    
    // Game control
    void newGame();
    void setGameMode(GameMode mode);
    void quit();
    
    // Event handling (delegates to InputHandler)
    void handleEvent(const SDL_Event& event);
    
    // Rendering (delegates to Renderer)
    void render();
};

#endif // GAME_CONTROLLER_H
