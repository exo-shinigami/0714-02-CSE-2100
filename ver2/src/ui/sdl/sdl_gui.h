#ifndef GUI_H
#define GUI_H

#include "types_definitions.h"
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

// Move history constants
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

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* pieceTextures[13];
    int selectedSquare;
    int isRunning;
    int gameOver;  // 0 = game active, 1 = game over
    char gameOverMessage[256];  // Message to display when game is over (increased buffer size)
    int gameMode;  // MODE_PVE or MODE_PVP
    
    // Move history tracking
    char moveHistory[MAX_DISPLAY_MOVES][10];  // Store move strings like "e2e4"
    int moveCount;
    int historyScrollOffset;
    
    // Chess timer tracking
    int whiteTimeMs;           // White's remaining time in milliseconds
    int blackTimeMs;           // Black's remaining time in milliseconds
    int incrementMs;           // Time increment per move
    int lastMoveTime;          // Timestamp of last move
    int timerActive;           // Is timer running (0=no, 1=yes)
    int timerPaused;           // Is timer paused
    
    // Move highlighting
    int possibleMoves[256];    // Array of possible move squares
    int possibleMovesCount;    // Number of possible moves
    
    // Pawn promotion
    int promotionPending;      // 1 if waiting for promotion choice
    int promotionFromSq;       // Source square for promotion
    int promotionToSq;         // Target square for promotion
} GUI;

// Function declarations
int GUI_Init(GUI* gui);
void CleanupGUI(GUI* gui);
void GUI_RenderBoard(GUI* gui, ChessBoard* board);
void RenderCapturedPieces(GUI* gui, ChessBoard* board);
void RenderMoveHistory(GUI* gui, ChessBoard* board);
void RenderTimers(GUI* gui, ChessBoard* board);
void AddMoveToHistory(GUI* gui, const char* moveStr);
void UpdateTimer(GUI* gui, ChessBoard* board);
void ResetTimers(GUI* gui);
void GUI_HandleMouseClick(GUI* gui, ChessBoard* board, SearchInfo* info, int x, int y);
int SquareFromCoords(int x, int y);
void GetSquareCoords(int square, int* x, int* y);
void GUI_DrawPiece(GUI* gui, int piece, int x, int y);
void DrawCapturedPiece(GUI* gui, int piece, int x, int y, int size);
void GUI_Run(ChessBoard* board, SearchInfo* info);
const char* GetPieceSymbol(int piece);

#endif