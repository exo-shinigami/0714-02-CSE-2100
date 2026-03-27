/**
 * @file sdl_gui.h
 * @brief SDL GUI composition root and legacy compatibility surface
 *
 * OOD/SOLID role:
 * - SRP: Aggregates GUI runtime state while delegating input/history/timer
 *   responsibilities to dedicated component classes.
 * - DIP: Uses forward declarations for high-level dependencies.
 * - ISP/OCP: Public API remains stable while internal components evolve.
 */
#ifndef GUI_H
#define GUI_H

#include "types_definitions.h"
#include "game_timer.h"
#include "move_history_tracker.h"
#include "gui_input_handler.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>  // Add explicit TTF include

#define BOARD_SIZE 640
#define SQUARE_SIZE 80
#define CAPTURED_PANEL_WIDTH 200
#define MOVE_HISTORY_WIDTH 240
#define WINDOW_WIDTH (BOARD_SIZE + CAPTURED_PANEL_WIDTH + MOVE_HISTORY_WIDTH)
#define WINDOW_HEIGHT 700

// Captured pieces display constants
#define CAPTURED_PIECE_SIZE 40
#define CAPTURED_PIECE_PADDING 5
#define CAPTURED_SECTION_Y_START 10

// move history constants
#define MAX_DISPLAY_MOVES 100
#define MOVE_HISTORY_SCROLL_SPEED 3

// Timer constants
#define DEFAULT_TIME_MS 600000  // 10 minutes in milliseconds
#define DEFAULT_INCREMENT_MS 0   // No increment by default

// Colors
#define WHITE_SQUARE_R 240
#define WHITE_SQUARE_G 217
#define WHITE_SQUARE_B 181

#define BLACK_SQUARE_R 181
#define BLACK_SQUARE_G 136
#define BLACK_SQUARE_B 99

#define HIGHLIGHT_R 255
#define HIGHLIGHT_G 255
#define HIGHLIGHT_B 0

// Game modes
#define MODE_PVE 0  // Player vs Engine
#define MODE_PVP 1  // Player vs Player

struct GUI {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* pieceTextures[13];
    int selectedSquare;
    int isRunning;
    int gameOver;  // 0 = game active, 1 = game over
    char gameOverMessage[256];  // Message to display when game is over (increased buffer size)
    int gameMode;  // MODE_PVE or MODE_PVP
    
    // move history tracking
    char moveHistory[MAX_DISPLAY_MOVES][10];  // Store move strings like "e2e4"
    int moveCount;
    int historyScrollOffset;
    MoveHistoryTracker moveHistoryTracker;
    
    // Chess timer tracking
    int whiteTimeMs;           // White's remaining time in milliseconds
    int blackTimeMs;           // Black's remaining time in milliseconds
    int incrementMs;           // Time increment per move
    int lastMoveTime;          // Timestamp of last move
    int timerActive;           // Is timer running (0=no, 1=yes)
    int timerPaused;           // Is timer paused
    GameTimer gameTimer;

    // Input handling abstraction
    GUIInputHandler inputHandler;
    
    // move highlighting
    int possibleMoves[256];    // Array of possible move squares
    int possibleMovesCount;    // Number of possible moves
    
    // Pawn promotion
    int promotionPending;      // 1 if waiting for promotion choice
    int promotionFromSq;       // Source square for promotion
    int promotionToSq;         // Target square for promotion

    // Captured pieces (SRP: tracked here, not in ChessBoard)
    int capturedWhite[16];
    int capturedBlack[16];
    int capturedWhiteCount;
    int capturedBlackCount;
};

// Function declarations
int gUIInit(GUI* gui);
void cleanupGUI(GUI* gui);
void gUIRenderBoard(GUI* gui, ChessBoard* board);
void renderCapturedPieces(GUI* gui);
void renderMoveHistory(GUI* gui, ChessBoard* board);
void renderTimers(GUI* gui, ChessBoard* board);
void addMoveToHistory(GUI* gui, const char* moveStr);
void updateTimer(GUI* gui, ChessBoard* board);
void resetTimers(GUI* gui);
void gUIHandleMouseClick(GUI* gui, ChessBoard* board, SearchInfo* info, int x, int y, const IEvaluator& eval);
int squareFromCoords(int x, int y);
void getSquareCoords(int square, int* x, int* y);
void gUIDrawPiece(GUI* gui, int piece, int x, int y);
void drawCapturedPiece(GUI* gui, int piece, int x, int y, int size);
void gUIRun(ChessBoard* board, SearchInfo* info, const IEvaluator& eval);
const char* GetPieceSymbol(int piece);

#endif