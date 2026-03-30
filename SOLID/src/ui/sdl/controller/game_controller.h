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
#include "../sdl_gui.h"
#include "../input/gui_input_handler.h"
#include "../services/engine_move_policy.h"
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
    const IEngineMovePolicy* movePolicy_;
    GUI gui_;
    GUIInputHandler inputHandler_;
    
    // Game state
    GameMode gameMode_;
    GameState gameState_;
    bool isRunning_;
    
    // Private helper methods
    void initializeGame();
    void applyInputAction(const InputEvent& inputEvent);
    void handleScrollEvent(int direction);
    void processSquareClick(int square);
    void resetForNewGame();
    void printHelp() const;
    
public:
    GameController(ChessBoard* board, SearchInfo* info, const IEvaluator& eval,
                   const IEngineMovePolicy& movePolicy = defaultEngineMovePolicy());
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
