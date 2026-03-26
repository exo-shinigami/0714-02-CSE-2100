/**
 * @file gui_renderer.h
 * @brief GUI Renderer - Handles all rendering operations (SRP)
 * 
 * This class follows Single Responsibility Principle by focusing only
 * on rendering operations. It does not handle input or game logic.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef GUI_RENDERER_H
#define GUI_RENDERER_H

#include "types_definitions.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * @class gUIRenderer
 * @brief Responsible only for rendering the chess GUI
 * 
 * SOLID Principles Applied:
 * - SRP: Only handles rendering operations
 * - OCP: Can extend rendering without modifying core logic
 * - DIP: Depends on ChessBoard abstraction, not internal implementation
 */
class gUIRenderer {
private:
    SDL_Renderer* renderer_;
    SDL_Texture* pieceTextures_[13];
    
    // Private rendering helpers
    void renderSquare(int square, bool isHighlighted, bool isSelected) const;
    void renderPieceAt(int piece, int x, int y) const;
    SDL_Texture* createPieceTexture(int piece);
    
public:
    explicit gUIRenderer(SDL_Renderer* renderer);
    ~gUIRenderer();
    
    // Non-copyable
    gUIRenderer(const gUIRenderer&) = delete;
    gUIRenderer& operator=(const gUIRenderer&) = delete;
    
    // Core rendering methods
    void renderBoard(const ChessBoard& board, int selectedSquare,
                     const int* possibleMoves, int possibleMovesCount) const;
    
    void renderCapturedPieces(const int* capturedWhite, int capturedWhiteCount,
                              const int* capturedBlack, int capturedBlackCount) const;
    
    void renderMoveHistory(const char moveHistory[][10], int moveCount, 
                          int scrollOffset) const;
    
    void renderTimers(int whiteTimeMs, int blackTimeMs, int currentSide) const;
    
    void renderGameOverMessage(const char* message) const;
    
    void renderPromotionDialog(int side) const;
    
    void renderStatusBar(const char* text) const;
    
    // Utility
    bool initializePieceTextures();
    void cleanup();
};

#endif // GUI_RENDERER_H
