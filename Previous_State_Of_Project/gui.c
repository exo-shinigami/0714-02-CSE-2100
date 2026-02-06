#include "gui.h"
#include "defs.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <ctype.h>

const char* GetPieceSymbol(int piece) {
    switch(piece) {
        case wP: return "♙";
        case wN: return "♘";
        case wB: return "♗";
        case wR: return "♖";
        case wQ: return "♕";
        case wK: return "♔";
        case bP: return "♟";
        case bN: return "♞";
        case bB: return "♝";
        case bR: return "♜";
        case bQ: return "♛";
        case bK: return "♚";
        default: return "";
    }
}

int InitGUI(GUI* gui) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 0;
    }

    gui->window = SDL_CreateWindow("Gambit Chess",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   SDL_WINDOW_SHOWN);

    if (gui->window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    gui->renderer = SDL_CreateRenderer(gui->window, -1, SDL_RENDERER_ACCELERATED);
    if (gui->renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    gui->selectedSquare = NO_SQ;
    gui->isRunning = 1;
    gui->gameOver = 0;
    strcpy(gui->gameOverMessage, "");
    gui->gameMode = MODE_PVP; // Default to Player vs Player for stability
    
    // Initialize move history
    gui->moveCount = 0;
    gui->historyScrollOffset = 0;
    for (int i = 0; i < MAX_DISPLAY_MOVES; i++) {
        strcpy(gui->moveHistory[i], "");
    }
    
    // Initialize chess timers
    gui->whiteTimeMs = DEFAULT_TIME_MS;
    gui->blackTimeMs = DEFAULT_TIME_MS;
    gui->incrementMs = DEFAULT_INCREMENT_MS;
    gui->lastMoveTime = 0;
    gui->timerActive = 0;
    gui->timerPaused = 0;
    
    // Initialize move highlighting
    gui->possibleMovesCount = 0;
    for (int i = 0; i < 256; i++) {
        gui->possibleMoves[i] = NO_SQ;
    }
    
    // Initialize pawn promotion
    gui->promotionPending = 0;
    gui->promotionFromSq = NO_SQ;
    gui->promotionToSq = NO_SQ;
    
    // Initialize piece textures array to NULL
    for (int i = 0; i < 13; i++) {
        gui->pieceTextures[i] = NULL;
    }

    return 1;
}

void CleanupGUI(GUI* gui) {
    if (!gui) return;  // Safety check for null pointer
    
    // Clean up piece textures if they exist
    for (int i = 0; i < 13; i++) {
        if (gui->pieceTextures[i]) {
            SDL_DestroyTexture(gui->pieceTextures[i]);
            gui->pieceTextures[i] = NULL;
        }
    }
    
    if (gui->renderer) {
        SDL_DestroyRenderer(gui->renderer);
        gui->renderer = NULL;
    }
    if (gui->window) {
        SDL_DestroyWindow(gui->window);
        gui->window = NULL;
    }
    TTF_Quit();
    SDL_Quit();
}

void RenderGameOverMessage(GUI* gui) {
    if (!gui->gameOver) return;
    
    // Create a semi-transparent overlay
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 180); // Semi-transparent black
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(gui->renderer, &overlay);
    
    // Load font (using a default system font)
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 36);
    if (!font) {
        // Fallback to any available font
        font = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 36);
    }
    if (!font) {
        // Try common cross-platform fonts
        font = TTF_OpenFont("arial.ttf", 36);
    }
    if (!font) {
        // Last resort - try system default
        font = TTF_OpenFont("/System/Library/Fonts/Arial.ttf", 36);
    }
    if (!font) {
        // If no fonts work, just display a simple rectangle with error logging
        printf("WARNING: Could not load any fonts for game over message\n");
        SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, 255);
        SDL_Rect messageBox = {200, 300, 400, 100};
        SDL_RenderFillRect(gui->renderer, &messageBox);
        return;
    }
    
    // Create text surface
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, gui->gameOverMessage, textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gui->renderer, textSurface);
        if (textTexture) {
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;
            
            // Center the text
            SDL_Rect textRect = {
                (WINDOW_WIDTH - textWidth) / 2,
                (WINDOW_HEIGHT - textHeight) / 2,
                textWidth,
                textHeight
            };
            
            SDL_RenderCopy(gui->renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
    
    TTF_CloseFont(font);
}

void RenderGameMode(GUI* gui) {
    // Load font for mode display
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 18);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 18);
    }
    if (!font) {
        font = TTF_OpenFont("arial.ttf", 18);
    }
    if (!font) {
        font = TTF_OpenFont("/System/Library/Fonts/Arial.ttf", 18);
    }
    if (!font) {
        printf("WARNING: Could not load fonts for game mode display\n");
        return; // Skip if no font available
    }
    
    // Create mode text
    const char* modeText = (gui->gameMode == MODE_PVE) ? "Mode: Player vs Engine" : "Mode: Player vs Player";
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, modeText, textColor);
    
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gui->renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect = {10, BOARD_SIZE + 10, textSurface->w, textSurface->h};
            SDL_RenderCopy(gui->renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
    
    // Add controls text
    const char* controlsText = "Controls: N=New Game, M=Switch Mode, H=Help";
    SDL_Surface* controlsSurface = TTF_RenderText_Solid(font, controlsText, textColor);
    
    if (controlsSurface) {
        SDL_Texture* controlsTexture = SDL_CreateTextureFromSurface(gui->renderer, controlsSurface);
        if (controlsTexture) {
            SDL_Rect controlsRect = {10, BOARD_SIZE + 35, controlsSurface->w, controlsSurface->h};
            SDL_RenderCopy(gui->renderer, controlsTexture, NULL, &controlsRect);
            SDL_DestroyTexture(controlsTexture);
        }
        SDL_FreeSurface(controlsSurface);
    }
    
    TTF_CloseFont(font);
}

void SetGameOver(GUI* gui, S_BOARD* pos) {
    gui->gameOver = 1;
    
    // Check for different types of draws first
    if (pos->fiftyMove > 100) {
        strcpy(gui->gameOverMessage, "DRAW! Fifty Move Rule");
        printf("DEBUG: Setting message - DRAW! Fifty Move Rule\n");
        return;
    }
    
    if (ThreeFoldRep(pos) >= 2) {
        strcpy(gui->gameOverMessage, "DRAW! Threefold Repetition");
        printf("DEBUG: Setting message - DRAW! Threefold Repetition\n");
        return;
    }
    
    if (DrawMaterial(pos) == TRUE) {
        strcpy(gui->gameOverMessage, "DRAW! Insufficient Material");
        printf("DEBUG: Setting message - DRAW! Insufficient Material\n");
        return;
    }
    
    // Determine the game over message based on the current position
    int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side^1, pos);
    
    printf("DEBUG GAME OVER: side=%s, InCheck=%d, KingSq=%s\n", 
           pos->side == WHITE ? "WHITE" : "BLACK", 
           InCheck, 
           PrSq(pos->KingSq[pos->side]));
    
    if (InCheck) {
        // Checkmate
        if (pos->side == WHITE) {
            strcpy(gui->gameOverMessage, "CHECKMATE! Black Wins!");
            printf("DEBUG: Setting message - CHECKMATE! Black Wins!\n");
        } else {
            strcpy(gui->gameOverMessage, "CHECKMATE! White Wins!");
            printf("DEBUG: Setting message - CHECKMATE! White Wins!\n");
        }
    } else {
        // Stalemate (no legal moves but not in check)
        strcpy(gui->gameOverMessage, "DRAW! Stalemate");
        printf("DEBUG: Setting message - DRAW! Stalemate\n");
    }
}

int IsPawnPromotion(S_BOARD* pos, int fromSq, int toSq) {
    int piece = pos->pieces[fromSq];
    
    // Check if it's a pawn
    if (!PiecePawn[piece]) return 0;
    
    // Check if moving to promotion rank
    int toRank = RanksBrd[toSq];
    if (piece == wP && toRank == RANK_8) return 1; // White pawn to 8th rank
    if (piece == bP && toRank == RANK_1) return 1; // Black pawn to 1st rank
    
    return 0;
}

char GetPromotionChoice() {
    // For now, default to Queen promotion to test the system
    // TODO: Replace with proper GUI dialog
    printf("PAWN PROMOTION: Automatically promoting to Queen\n");
    return 'q';
}

void RenderPromotionDialog(GUI* gui, S_BOARD* pos) {
    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(gui->renderer, &overlay);
    
    // Dialog box
    int dialogW = 400;
    int dialogH = 250;
    int dialogX = (WINDOW_WIDTH - dialogW) / 2;
    int dialogY = (WINDOW_HEIGHT - dialogH) / 2;
    
    SDL_SetRenderDrawColor(gui->renderer, 60, 60, 60, 255);
    SDL_Rect dialogBg = {dialogX, dialogY, dialogW, dialogH};
    SDL_RenderFillRect(gui->renderer, &dialogBg);
    
    SDL_SetRenderDrawColor(gui->renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(gui->renderer, &dialogBg);
    
    // Title
    TTF_Font* titleFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    if (titleFont) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* titleSurf = TTF_RenderText_Solid(titleFont, "Choose Promotion Piece", white);
        if (titleSurf) {
            SDL_Texture* titleTex = SDL_CreateTextureFromSurface(gui->renderer, titleSurf);
            if (titleTex) {
                SDL_Rect titleRect = {dialogX + (dialogW - titleSurf->w) / 2, dialogY + 15, titleSurf->w, titleSurf->h};
                SDL_RenderCopy(gui->renderer, titleTex, NULL, &titleRect);
                SDL_DestroyTexture(titleTex);
            }
            SDL_FreeSurface(titleSurf);
        }
        TTF_CloseFont(titleFont);
    }
    
    // Determine which color pieces to show
    int isWhite = (pos->side == BLACK); // After move, side switches, so we check opposite
    
    // Draw 4 piece options (Queen, Rook, Bishop, Knight)
    int pieceSize = 80;
    int spacing = 20;
    int startX = dialogX + (dialogW - (4 * pieceSize + 3 * spacing)) / 2;
    int startY = dialogY + 70;
    
    int pieces[4];
    if (isWhite) {
        pieces[0] = wQ; pieces[1] = wR; pieces[2] = wB; pieces[3] = wN;
    } else {
        pieces[0] = bQ; pieces[1] = bR; pieces[2] = bB; pieces[3] = bN;
    }
    
    // Draw piece boxes and pieces
    TTF_Font* pieceFont = TTF_OpenFont("C:/Windows/Fonts/seguisym.ttf", 50);
    if (!pieceFont) pieceFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 50);
    
    for (int i = 0; i < 4; i++) {
        int x = startX + i * (pieceSize + spacing);
        
        // Draw box
        SDL_SetRenderDrawColor(gui->renderer, 100, 100, 100, 255);
        SDL_Rect box = {x, startY, pieceSize, pieceSize};
        SDL_RenderFillRect(gui->renderer, &box);
        
        SDL_SetRenderDrawColor(gui->renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(gui->renderer, &box);
        
        // Draw piece symbol
        if (pieceFont) {
            const char* symbol = GetPieceSymbol(pieces[i]);
            SDL_Color color = isWhite ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){50, 50, 50, 255};
            SDL_Surface* pieceSurf = TTF_RenderUTF8_Blended(pieceFont, symbol, color);
            if (pieceSurf) {
                SDL_Texture* pieceTex = SDL_CreateTextureFromSurface(gui->renderer, pieceSurf);
                if (pieceTex) {
                    int offsetX = (pieceSize - pieceSurf->w) / 2;
                    int offsetY = (pieceSize - pieceSurf->h) / 2;
                    SDL_Rect pieceRect = {x + offsetX, startY + offsetY, pieceSurf->w, pieceSurf->h};
                    SDL_RenderCopy(gui->renderer, pieceTex, NULL, &pieceRect);
                    SDL_DestroyTexture(pieceTex);
                }
                SDL_FreeSurface(pieceSurf);
            }
        }
    }
    
    if (pieceFont) TTF_CloseFont(pieceFont);
    
    // Draw labels
    TTF_Font* labelFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 14);
    if (labelFont) {
        const char* labels[4] = {"Queen", "Rook", "Bishop", "Knight"};
        SDL_Color white = {255, 255, 255, 255};
        
        for (int i = 0; i < 4; i++) {
            int x = startX + i * (pieceSize + spacing);
            SDL_Surface* labelSurf = TTF_RenderText_Solid(labelFont, labels[i], white);
            if (labelSurf) {
                SDL_Texture* labelTex = SDL_CreateTextureFromSurface(gui->renderer, labelSurf);
                if (labelTex) {
                    SDL_Rect labelRect = {x + (pieceSize - labelSurf->w) / 2, startY + pieceSize + 10, labelSurf->w, labelSurf->h};
                    SDL_RenderCopy(gui->renderer, labelTex, NULL, &labelRect);
                    SDL_DestroyTexture(labelTex);
                }
                SDL_FreeSurface(labelSurf);
            }
        }
        TTF_CloseFont(labelFont);
    }
}

char HandlePromotionClick(GUI* gui, int mouseX, int mouseY) {
    int dialogW = 400;
    int dialogH = 250;
    int dialogX = (WINDOW_WIDTH - dialogW) / 2;
    int dialogY = (WINDOW_HEIGHT - dialogH) / 2;
    
    int pieceSize = 80;
    int spacing = 20;
    int startX = dialogX + (dialogW - (4 * pieceSize + 3 * spacing)) / 2;
    int startY = dialogY + 70;
    
    // Check which piece was clicked
    for (int i = 0; i < 4; i++) {
        int x = startX + i * (pieceSize + spacing);
        if (mouseX >= x && mouseX < x + pieceSize && mouseY >= startY && mouseY < startY + pieceSize) {
            char choices[4] = {'q', 'r', 'b', 'n'};
            return choices[i];
        }
    }
    
    return '\0'; // No selection
}

int SquareFromCoords(int x, int y) {
    // Enhanced bounds checking
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return NO_SQ;
    }
    
    int file = x / SQUARE_SIZE;
    int rank = 7 - (y / SQUARE_SIZE);
    
    // Additional safety check for file and rank bounds
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return NO_SQ;
    }
    
    return FR2SQ(file, rank);
}

void GetSquareCoords(int square, int* x, int* y) {
    // Safety check for null pointers
    if (!x || !y) return;
    
    // Safety check for valid square
    if (square == NO_SQ || square < 0 || square >= 120) {
        *x = 0;
        *y = 0;
        return;
    }
    
    int file = FilesBrd[square];
    int rank = RanksBrd[square];
    
    *x = file * SQUARE_SIZE;
    *y = (7 - rank) * SQUARE_SIZE;
}

void DrawPiece(GUI* gui, int piece, int x, int y) {
    // Safety check for piece bounds
    if (piece == EMPTY || piece == OFFBOARD || piece < 0 || piece >= 13) {
        return;
    }
    
    // Load font for chess symbols
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/seguisym.ttf", 60);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 60);
    }
    if (!font) return;
    
    // Get the Unicode chess symbol
    const char* symbol = GetPieceSymbol(piece);
    if (!symbol || symbol[0] == '\0') {
        TTF_CloseFont(font);
        return;
    }
    
    // Set color based on piece type
    SDL_Color color;
    if (piece >= wP && piece <= wK) {
        // White pieces
        color.r = 255;
        color.g = 255;
        color.b = 255;
        color.a = 255;
    } else {
        // Black pieces
        color.r = 50;
        color.g = 50;
        color.b = 50;
        color.a = 255;
    }
    
    // Render the chess symbol
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, symbol, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
        if (texture) {
            // Center the symbol in the square
            int offsetX = (SQUARE_SIZE - surface->w) / 2;
            int offsetY = (SQUARE_SIZE - surface->h) / 2;
            SDL_Rect rect = {x + offsetX, y + offsetY, surface->w, surface->h};
            SDL_RenderCopy(gui->renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    
    TTF_CloseFont(font);
}

void DrawCapturedPiece(GUI* gui, int piece, int x, int y, int size) {
    // Safety check for piece bounds
    if (piece == EMPTY || piece == OFFBOARD || piece < 0 || piece >= 13) {
        return;
    }
    
    // Load font for chess symbols (smaller size for captured pieces)
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/seguisym.ttf", size - 5);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", size - 5);
    }
    if (!font) return;
    
    // Get the Unicode chess symbol
    const char* symbol = GetPieceSymbol(piece);
    if (!symbol || symbol[0] == '\0') {
        TTF_CloseFont(font);
        return;
    }
    
    // Set color based on piece type
    SDL_Color color;
    if (piece >= wP && piece <= wK) {
        // White pieces
        color.r = 255;
        color.g = 255;
        color.b = 255;
        color.a = 255;
    } else {
        // Black pieces
        color.r = 50;
        color.g = 50;
        color.b = 50;
        color.a = 255;
    }
    
    // Render the chess symbol
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, symbol, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
        if (texture) {
            // Center the symbol in the capture box
            int offsetX = (size - surface->w) / 2;
            int offsetY = (size - surface->h) / 2;
            SDL_Rect rect = {x + offsetX, y + offsetY, surface->w, surface->h};
            SDL_RenderCopy(gui->renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    
    TTF_CloseFont(font);
}

void RenderCapturedPieces(GUI* gui, S_BOARD* pos) {
    int panelX = BOARD_SIZE;
    
    // Draw panel background
    SDL_SetRenderDrawColor(gui->renderer, 40, 40, 40, 255);
    SDL_Rect panel = {panelX, 0, CAPTURED_PANEL_WIDTH, BOARD_SIZE};
    SDL_RenderFillRect(gui->renderer, &panel);
    
    // Draw separator line
    SDL_SetRenderDrawColor(gui->renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(gui->renderer, panelX, 0, panelX, BOARD_SIZE);
    
    // Load font for labels
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 16);
    }
    
    // Render "Captured Pieces" title
    if (font) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Captured", textColor);
        if (titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(gui->renderer, titleSurface);
            if (titleTexture) {
                SDL_Rect titleRect = {panelX + (CAPTURED_PANEL_WIDTH - titleSurface->w) / 2, 5, titleSurface->w, titleSurface->h};
                SDL_RenderCopy(gui->renderer, titleTexture, NULL, &titleRect);
                SDL_DestroyTexture(titleTexture);
            }
            SDL_FreeSurface(titleSurface);
        }
    }
    
    int startY = CAPTURED_SECTION_Y_START + 30;
    int columnWidth = CAPTURED_PANEL_WIDTH / 2;
    int blackColumnX = panelX + 10;
    int whiteColumnX = panelX + columnWidth + 5;
    
    // Render BLACK pieces label and pieces (left column)
    if (font) {
        SDL_Color textColor = {200, 200, 200, 255};
        SDL_Surface* labelSurface = TTF_RenderText_Solid(font, "Black", textColor);
        if (labelSurface) {
            SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(gui->renderer, labelSurface);
            if (labelTexture) {
                SDL_Rect labelRect = {blackColumnX, startY, labelSurface->w, labelSurface->h};
                SDL_RenderCopy(gui->renderer, labelTexture, NULL, &labelRect);
                SDL_DestroyTexture(labelTexture);
            }
            SDL_FreeSurface(labelSurface);
        }
    }
    
    int blackY = startY + 25;
    for (int i = 0; i < pos->capturedBlackCount; i++) {
        int piece = pos->capturedBlack[i];
        if (piece != EMPTY && piece >= bP && piece <= bK) {
            DrawCapturedPiece(gui, piece, blackColumnX, blackY, CAPTURED_PIECE_SIZE);
            blackY += CAPTURED_PIECE_SIZE + CAPTURED_PIECE_PADDING;
            // Prevent overflow
            if (blackY + CAPTURED_PIECE_SIZE > BOARD_SIZE - 10) break;
        }
    }
    
    // Render WHITE pieces label and pieces (right column)
    if (font) {
        SDL_Color textColor = {200, 200, 200, 255};
        SDL_Surface* labelSurface = TTF_RenderText_Solid(font, "White", textColor);
        if (labelSurface) {
            SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(gui->renderer, labelSurface);
            if (labelTexture) {
                SDL_Rect labelRect = {whiteColumnX, startY, labelSurface->w, labelSurface->h};
                SDL_RenderCopy(gui->renderer, labelTexture, NULL, &labelRect);
                SDL_DestroyTexture(labelTexture);
            }
            SDL_FreeSurface(labelSurface);
        }
    }
    
    int whiteY = startY + 25;
    for (int i = 0; i < pos->capturedWhiteCount; i++) {
        int piece = pos->capturedWhite[i];
        if (piece != EMPTY && piece >= wP && piece <= wK) {
            DrawCapturedPiece(gui, piece, whiteColumnX, whiteY, CAPTURED_PIECE_SIZE);
            whiteY += CAPTURED_PIECE_SIZE + CAPTURED_PIECE_PADDING;
            // Prevent overflow
            if (whiteY + CAPTURED_PIECE_SIZE > BOARD_SIZE - 10) break;
        }
    }
    
    if (font) {
        TTF_CloseFont(font);
    }
}

void AddMoveToHistory(GUI* gui, const char* moveStr) {
    if (gui->moveCount < MAX_DISPLAY_MOVES) {
        strncpy(gui->moveHistory[gui->moveCount], moveStr, 9);
        gui->moveHistory[gui->moveCount][9] = '\0';
        gui->moveCount++;
    }
}

void ResetTimers(GUI* gui) {
    gui->whiteTimeMs = DEFAULT_TIME_MS;
    gui->blackTimeMs = DEFAULT_TIME_MS;
    gui->lastMoveTime = GetTimeMs();
    gui->timerActive = 1;
    gui->timerPaused = 0;
}

void UpdateTimer(GUI* gui, S_BOARD* pos) {
    if (!gui->timerActive || gui->timerPaused || gui->gameOver) {
        return;
    }
    
    int currentTime = GetTimeMs();
    int elapsed = currentTime - gui->lastMoveTime;
    gui->lastMoveTime = currentTime;
    
    // Deduct time from the side to move
    if (pos->side == WHITE) {
        gui->whiteTimeMs -= elapsed;
        if (gui->whiteTimeMs < 0) {
            gui->whiteTimeMs = 0;
            gui->gameOver = 1;
            strcpy(gui->gameOverMessage, "TIME OUT! Black Wins!");
            gui->timerActive = 0;
        }
    } else {
        gui->blackTimeMs -= elapsed;
        if (gui->blackTimeMs < 0) {
            gui->blackTimeMs = 0;
            gui->gameOver = 1;
            strcpy(gui->gameOverMessage, "TIME OUT! White Wins!");
            gui->timerActive = 0;
        }
    }
}

void RenderTimers(GUI* gui, S_BOARD* pos) {
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 24);
    }
    if (!font) return;
    
    TTF_Font* labelFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
    if (!labelFont) {
        labelFont = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 16);
    }
    
    // Calculate positions - bottom right corner, side by side within status area
    int timerY = BOARD_SIZE + 8;
    int moveHistoryPanelX = BOARD_SIZE + CAPTURED_PANEL_WIDTH;
    int whiteTimerX = moveHistoryPanelX + 5;
    int blackTimerX = moveHistoryPanelX + 125;  // Side by side with spacing
    
    // Draw background boxes (smaller to fit side by side)
    SDL_SetRenderDrawColor(gui->renderer, 40, 40, 40, 255);
    SDL_Rect whiteTimerBox = {whiteTimerX - 3, timerY - 3, 110, 46};
    SDL_Rect blackTimerBox = {blackTimerX - 3, timerY - 3, 110, 46};
    SDL_RenderFillRect(gui->renderer, &whiteTimerBox);
    SDL_RenderFillRect(gui->renderer, &blackTimerBox);
    
    // Highlight active player's timer
    if (pos->side == WHITE) {
        SDL_SetRenderDrawColor(gui->renderer, 100, 200, 100, 255);
        SDL_RenderDrawRect(gui->renderer, &whiteTimerBox);
        SDL_RenderDrawRect(gui->renderer, &(SDL_Rect){whiteTimerBox.x - 1, whiteTimerBox.y - 1, whiteTimerBox.w + 2, whiteTimerBox.h + 2});
    } else {
        SDL_SetRenderDrawColor(gui->renderer, 100, 200, 100, 255);
        SDL_RenderDrawRect(gui->renderer, &blackTimerBox);
        SDL_RenderDrawRect(gui->renderer, &(SDL_Rect){blackTimerBox.x - 1, blackTimerBox.y - 1, blackTimerBox.w + 2, blackTimerBox.h + 2});
    }
    
    // Format time strings (MM:SS)
    char whiteTimeStr[16];
    char blackTimeStr[16];
    int whiteMinutes = gui->whiteTimeMs / 60000;
    int whiteSeconds = (gui->whiteTimeMs % 60000) / 1000;
    int blackMinutes = gui->blackTimeMs / 60000;
    int blackSeconds = (gui->blackTimeMs % 60000) / 1000;
    
    snprintf(whiteTimeStr, sizeof(whiteTimeStr), "%02d:%02d", whiteMinutes, whiteSeconds);
    snprintf(blackTimeStr, sizeof(blackTimeStr), "%02d:%02d", blackMinutes, blackSeconds);
    
    // Render White timer
    SDL_Color whiteColor = {255, 255, 255, 255};
    if (labelFont) {
        SDL_Surface* labelSurface = TTF_RenderText_Solid(labelFont, "White", whiteColor);
        if (labelSurface) {
            SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(gui->renderer, labelSurface);
            if (labelTexture) {
                SDL_Rect labelRect = {whiteTimerX, timerY, labelSurface->w, labelSurface->h};
                SDL_RenderCopy(gui->renderer, labelTexture, NULL, &labelRect);
                SDL_DestroyTexture(labelTexture);
            }
            SDL_FreeSurface(labelSurface);
        }
    }
    
    SDL_Surface* whiteSurface = TTF_RenderText_Solid(font, whiteTimeStr, whiteColor);
    if (whiteSurface) {
        SDL_Texture* whiteTexture = SDL_CreateTextureFromSurface(gui->renderer, whiteSurface);
        if (whiteTexture) {
            SDL_Rect whiteRect = {whiteTimerX, timerY + 20, whiteSurface->w, whiteSurface->h};
            SDL_RenderCopy(gui->renderer, whiteTexture, NULL, &whiteRect);
            SDL_DestroyTexture(whiteTexture);
        }
        SDL_FreeSurface(whiteSurface);
    }
    
    // Render Black timer
    SDL_Color blackColor = {200, 200, 200, 255};
    if (labelFont) {
        SDL_Surface* labelSurface = TTF_RenderText_Solid(labelFont, "Black", blackColor);
        if (labelSurface) {
            SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(gui->renderer, labelSurface);
            if (labelTexture) {
                SDL_Rect labelRect = {blackTimerX, timerY, labelSurface->w, labelSurface->h};
                SDL_RenderCopy(gui->renderer, labelTexture, NULL, &labelRect);
                SDL_DestroyTexture(labelTexture);
            }
            SDL_FreeSurface(labelSurface);
        }
    }
    
    SDL_Surface* blackSurface = TTF_RenderText_Solid(font, blackTimeStr, blackColor);
    if (blackSurface) {
        SDL_Texture* blackTexture = SDL_CreateTextureFromSurface(gui->renderer, blackSurface);
        if (blackTexture) {
            SDL_Rect blackRect = {blackTimerX, timerY + 20, blackSurface->w, blackSurface->h};
            SDL_RenderCopy(gui->renderer, blackTexture, NULL, &blackRect);
            SDL_DestroyTexture(blackTexture);
        }
        SDL_FreeSurface(blackSurface);
    }
    
    TTF_CloseFont(font);
    if (labelFont) {
        TTF_CloseFont(labelFont);
    }
}

void RenderMoveHistory(GUI* gui, S_BOARD* pos) {
    int panelX = BOARD_SIZE + CAPTURED_PANEL_WIDTH;
    
    // Draw panel background
    SDL_SetRenderDrawColor(gui->renderer, 35, 35, 35, 255);
    SDL_Rect panel = {panelX, 0, MOVE_HISTORY_WIDTH, BOARD_SIZE};
    SDL_RenderFillRect(gui->renderer, &panel);
    
    // Draw separator line
    SDL_SetRenderDrawColor(gui->renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(gui->renderer, panelX, 0, panelX, BOARD_SIZE);
    
    // Load font for moves
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 14);
    if (!font) {
        font = TTF_OpenFont("C:/Windows/Fonts/calibri.ttf", 14);
    }
    if (!font) return;
    
    // Render title
    SDL_Color titleColor = {255, 255, 255, 255};
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Move History", titleColor);
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(gui->renderer, titleSurface);
        if (titleTexture) {
            SDL_Rect titleRect = {panelX + 10, 10, titleSurface->w, titleSurface->h};
            SDL_RenderCopy(gui->renderer, titleTexture, NULL, &titleRect);
            SDL_DestroyTexture(titleTexture);
        }
        SDL_FreeSurface(titleSurface);
    }
    
    // Render moves in pairs (White | Black)
    int startY = 40;
    int lineHeight = 22;
    int maxVisibleMoves = (BOARD_SIZE - startY - 10) / lineHeight;
    int startIndex = gui->historyScrollOffset;
    
    SDL_Color textColor = {200, 200, 200, 255};
    SDL_Color moveNumColor = {150, 150, 150, 255};
    
    for (int i = startIndex; i < gui->moveCount && (i - startIndex) * lineHeight < maxVisibleMoves * lineHeight; i++) {
        char displayText[50];
        
        // Display move pairs: 1. e2e4 e7e5
        if (i % 2 == 0) {
            int moveNum = (i / 2) + 1;
            if (i + 1 < gui->moveCount) {
                snprintf(displayText, sizeof(displayText), "%d. %s %s", 
                        moveNum, gui->moveHistory[i], gui->moveHistory[i + 1]);
            } else {
                snprintf(displayText, sizeof(displayText), "%d. %s", 
                        moveNum, gui->moveHistory[i]);
            }
            
            SDL_Surface* moveSurface = TTF_RenderText_Solid(font, displayText, textColor);
            if (moveSurface) {
                SDL_Texture* moveTexture = SDL_CreateTextureFromSurface(gui->renderer, moveSurface);
                if (moveTexture) {
                    int yPos = startY + ((i / 2) - startIndex / 2) * lineHeight;
                    SDL_Rect moveRect = {panelX + 10, yPos, moveSurface->w, moveSurface->h};
                    SDL_RenderCopy(gui->renderer, moveTexture, NULL, &moveRect);
                    SDL_DestroyTexture(moveTexture);
                }
                SDL_FreeSurface(moveSurface);
            }
        }
    }
    
    TTF_CloseFont(font);
}

void RenderBoard(GUI* gui, S_BOARD* pos) {
    SDL_SetRenderDrawColor(gui->renderer, 50, 50, 50, 255);
    SDL_RenderClear(gui->renderer);
    
    // Check if king is in check
    int kingInCheck = SqAttacked(pos->KingSq[pos->side], pos->side^1, pos);
    int kingSquare = pos->KingSq[pos->side];
    
    // Draw board squares
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int x = file * SQUARE_SIZE;
            int y = rank * SQUARE_SIZE;
            
            SDL_Rect square = {x, y, SQUARE_SIZE, SQUARE_SIZE};
            
            int sq120 = FR2SQ(file, 7 - rank);
            
            // Determine square color
            if ((file + rank) % 2 == 0) {
                SDL_SetRenderDrawColor(gui->renderer, WHITE_SQUARE_R, WHITE_SQUARE_G, WHITE_SQUARE_B, 255);
            } else {
                SDL_SetRenderDrawColor(gui->renderer, BLACK_SQUARE_R, BLACK_SQUARE_G, BLACK_SQUARE_B, 255);
            }
            
            // Highlight king in check (red)
            if (kingInCheck && sq120 == kingSquare) {
                SDL_SetRenderDrawColor(gui->renderer, 255, 50, 50, 255);
            }
            // Highlight selected square (yellow)
            else if (sq120 == gui->selectedSquare) {
                SDL_SetRenderDrawColor(gui->renderer, HIGHLIGHT_R, HIGHLIGHT_G, HIGHLIGHT_B, 255);
            }
            // Highlight possible move squares (green with transparency)
            else {
                int isPossibleMove = 0;
                for (int i = 0; i < gui->possibleMovesCount; i++) {
                    if (gui->possibleMoves[i] == sq120) {
                        isPossibleMove = 1;
                        break;
                    }
                }
                if (isPossibleMove) {
                    // Draw base square color first
                    SDL_RenderFillRect(gui->renderer, &square);
                    // Then draw green highlight with transparency
                    SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(gui->renderer, 0, 255, 0, 100);
                }
            }
            
            SDL_RenderFillRect(gui->renderer, &square);
            
            // Reset blend mode
            SDL_SetRenderDrawBlendMode(gui->renderer, SDL_BLENDMODE_NONE);
            
            // Draw piece if present
            if (pos->pieces[sq120] != EMPTY && pos->pieces[sq120] != OFFBOARD) {
                DrawPiece(gui, pos->pieces[sq120], x, y);
            }
        }
    }
    
    // Draw board border
    SDL_SetRenderDrawColor(gui->renderer, 100, 100, 100, 255);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {-i, -i, BOARD_SIZE + 2*i, BOARD_SIZE + 2*i};
        SDL_RenderDrawRect(gui->renderer, &border);
    }
    
    // Draw captured pieces panel
    RenderCapturedPieces(gui, pos);
    
    // Draw move history panel
    RenderMoveHistory(gui, pos);
    
    // Draw status text area
    SDL_SetRenderDrawColor(gui->renderer, 30, 30, 30, 255);
    SDL_Rect statusArea = {0, BOARD_SIZE, WINDOW_WIDTH, WINDOW_HEIGHT - BOARD_SIZE};
    SDL_RenderFillRect(gui->renderer, &statusArea);
    
    // Draw timers
    RenderTimers(gui, pos);
    
    // Draw game mode and controls
    RenderGameMode(gui);
    
    // Draw game over message if applicable
    RenderGameOverMessage(gui);
    
    // Draw promotion dialog if pending
    if (gui->promotionPending) {
        RenderPromotionDialog(gui, pos);
    }
    
    SDL_RenderPresent(gui->renderer);
}

void CalculatePossibleMoves(GUI* gui, S_BOARD* pos, int fromSquare) {
    gui->possibleMovesCount = 0;
    
    // Generate all legal moves
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    
    // Filter moves that start from the selected square
    for (int i = 0; i < list->count; i++) {
        int move = list->moves[i].move;
        
        // Check if move is legal by trying it
        if (MakeMove(pos, move)) {
            int from = FROMSQ(move);
            int to = TOSQ(move);
            
            TakeMove(pos); // Undo the move
            
            // If this move starts from our selected square, add destination to possible moves
            if (from == fromSquare) {
                gui->possibleMoves[gui->possibleMovesCount++] = to;
            }
        }
    }
}

void HandleMouseClick(GUI* gui, S_BOARD* pos, S_SEARCHINFO* info, int x, int y) {
    // Handle promotion dialog click
    if (gui->promotionPending) {
        char choice = HandlePromotionClick(gui, x, y);
        if (choice != '\0') {
            // Build the promotion move string
            char moveStr[8];
            snprintf(moveStr, sizeof(moveStr), "%c%c%c%c%c", 
                   FileChar[FilesBrd[gui->promotionFromSq]], 
                   RankChar[RanksBrd[gui->promotionFromSq]],
                   FileChar[FilesBrd[gui->promotionToSq]], 
                   RankChar[RanksBrd[gui->promotionToSq]],
                   choice);
            
            printf("Pawn promotion move: %s\n", moveStr);
            
            int move = ParseMove(moveStr, pos);
            if (move != NOMOVE && MakeMove(pos, move)) {
                gui->promotionPending = 0;
                gui->promotionFromSq = NO_SQ;
                gui->promotionToSq = NO_SQ;
                gui->selectedSquare = NO_SQ;
                gui->possibleMovesCount = 0;
                
                // Add move to history
                AddMoveToHistory(gui, moveStr);
                
                // Timer handling
                if (!gui->timerActive) {
                    gui->timerActive = 1;
                    gui->lastMoveTime = GetTimeMs();
                }
                if (pos->side == WHITE) {
                    gui->blackTimeMs += gui->incrementMs;
                } else {
                    gui->whiteTimeMs += gui->incrementMs;
                }
                gui->lastMoveTime = GetTimeMs();
                
                // Check for game over
                if (checkresult(pos) == TRUE) {
                    SetGameOver(gui, pos);
                    return;
                }
                
                // Engine move in PvE mode
                if (gui->gameMode == MODE_PVE) {
                    S_MOVELIST list[1];
                    GenerateAllMoves(pos, list);
                    
                    int engineMove = NOMOVE;
                    int bestScore = -999999;
                    
                    if (list->count > 0) {
                        for (int i = 0; i < list->count; i++) {
                            int move = list->moves[i].move;
                            if (MakeMove(pos, move)) {
                                int score = -EvalPosition(pos);
                                TakeMove(pos);
                                if (score > bestScore) {
                                    bestScore = score;
                                    engineMove = move;
                                }
                            }
                        }
                    }
                    
                    if (engineMove != NOMOVE) {
                        char engineMoveStr[10];
                        snprintf(engineMoveStr, sizeof(engineMoveStr), "%s", PrMove(engineMove));
                        AddMoveToHistory(gui, engineMoveStr);
                        MakeMove(pos, engineMove);
                        
                        if (pos->side == WHITE) {
                            gui->blackTimeMs += gui->incrementMs;
                        } else {
                            gui->whiteTimeMs += gui->incrementMs;
                        }
                        gui->lastMoveTime = GetTimeMs();
                        
                        if (checkresult(pos) == TRUE) {
                            SetGameOver(gui, pos);
                        }
                    }
                }
            }
        }
        return;
    }
    
    // Don't process clicks if game is over
    if (gui->gameOver) {
        printf("Game is over! No more moves allowed.\n");
        return;
    }
    
    // Check for checkmate/stalemate at the start of each turn
    printf("DEBUG: Checking game status with checkresult()...\n");
    if (checkresult(pos) == TRUE) {
        SetGameOver(gui, pos);
        printf("*** GAME OVER - No legal moves available ***\n");
        return;
    }
    printf("DEBUG: checkresult() returned FALSE - game continues\n");
    
    int clickedSquare = SquareFromCoords(x, y);
    
    if (clickedSquare == NO_SQ) return;
    
    printf("\n=== MOUSE CLICK DEBUG ===\n");
    printf("Mouse coords: (%d, %d)\n", x, y);
    printf("Clicked square: %s (sq120: %d)\n", PrSq(clickedSquare), clickedSquare);
    printf("Current side to move: %s (%d)\n", pos->side == WHITE ? "WHITE" : "BLACK", pos->side);
    printf("Selected square: %s\n", gui->selectedSquare == NO_SQ ? "NONE" : PrSq(gui->selectedSquare));
    
    if (gui->selectedSquare == NO_SQ) {
        // Select a piece
        if (pos->pieces[clickedSquare] != EMPTY && pos->pieces[clickedSquare] != OFFBOARD) {
            // Check if it's the right color to move
            int piece = pos->pieces[clickedSquare];
            printf("Piece at square: %d (%c), side to move: %d\n", piece, PceChar[piece], pos->side);
            printf("Piece color check: WHITE pieces %d-%d, BLACK pieces %d-%d\n", wP, wK, bP, bK);
            
            if ((pos->side == WHITE && piece >= wP && piece <= wK) ||
                (pos->side == BLACK && piece >= bP && piece <= bK)) {
                gui->selectedSquare = clickedSquare;
                CalculatePossibleMoves(gui, pos, clickedSquare);
                printf("✓ Selected piece %c at %s with %d possible moves\n", PceChar[piece], PrSq(clickedSquare), gui->possibleMovesCount);
            } else {
                printf("✗ Wrong color piece (piece=%d, side=%d) or not your turn\n", piece, pos->side);
            }
        } else {
            printf("✗ No piece at clicked square (piece=%d)\n", pos->pieces[clickedSquare]);
        }
    } else {
        // Try to make a move
        if (clickedSquare == gui->selectedSquare) {
            // Deselect
            gui->selectedSquare = NO_SQ;
            gui->possibleMovesCount = 0;
            printf("✗ Deselected piece\n");
        } else {
            // Attempt move
            char moveStr[8];
            
            // Check if this is a pawn promotion
            if (IsPawnPromotion(pos, gui->selectedSquare, clickedSquare)) {
                // Set up promotion dialog
                gui->promotionPending = 1;
                gui->promotionFromSq = gui->selectedSquare;
                gui->promotionToSq = clickedSquare;
                printf("Pawn promotion pending - waiting for user choice\n");
                return;
            }
            
            // Build move string
            snprintf(moveStr, sizeof(moveStr), "%c%c%c%c", 
                   FileChar[FilesBrd[gui->selectedSquare]], 
                   RankChar[RanksBrd[gui->selectedSquare]],
                   FileChar[FilesBrd[clickedSquare]], 
                   RankChar[RanksBrd[clickedSquare]]);
            
            printf("Attempting move: %s (from %s to %s)\n", moveStr, PrSq(gui->selectedSquare), PrSq(clickedSquare));
            printf("Side before move: %s (%d)\n", pos->side == WHITE ? "WHITE" : "BLACK", pos->side);
            
            int move = ParseMove(moveStr, pos);
            if (move != NOMOVE) {
                printf("✓ Valid move found (moveInt=%d), making move...\n", move);
                printf("DEBUG: Before MakeMove - side=%d (%s)\n", pos->side, pos->side == WHITE ? "WHITE" : "BLACK");
                int makeMovResult = MakeMove(pos, move);
                printf("DEBUG: MakeMove returned: %d\n", makeMovResult);
                printf("DEBUG: After MakeMove - side=%d (%s)\n", pos->side, pos->side == WHITE ? "WHITE" : "BLACK");
                
                if (makeMovResult) {
                    printf("Side after user move: %s (%d)\n", pos->side == WHITE ? "WHITE" : "BLACK", pos->side);
                    gui->selectedSquare = NO_SQ;
                    gui->possibleMovesCount = 0;
                    
                    // Start timer on first move
                    if (!gui->timerActive) {
                        gui->timerActive = 1;
                        gui->lastMoveTime = GetTimeMs();
                    }
                    
                    // Add move to history
                    AddMoveToHistory(gui, moveStr);
                    
                    // Add increment to previous player's time
                    if (pos->side == WHITE) {
                        // Black just moved
                        gui->blackTimeMs += gui->incrementMs;
                    } else {
                        // White just moved
                        gui->whiteTimeMs += gui->incrementMs;
                    }
                    gui->lastMoveTime = GetTimeMs();
                
                    // Check board validity
                    if (!CheckBoard(pos)) {
                        printf("✗ Board check failed after move\n");
                        return;
                    }
                    
                    // Check for checkmate/stalemate after user move
                    printf("DEBUG: About to call checkresult after move...\n");
                    if (checkresult(pos) == TRUE) {
                        printf("DEBUG: checkresult returned TRUE - calling SetGameOver\n");
                        SetGameOver(gui, pos);
                        printf("*** GAME OVER ***\n");
                        return; // Game has ended
                    }
                    printf("DEBUG: checkresult returned FALSE after move\n");
                    
                    // Only let engine move in PvE mode
                    if (gui->gameMode == MODE_PVE) {
                        // Engine vs Player mode - let engine make a move
                        if (pos->side == WHITE || pos->side == BLACK) {
                            // Generate all legal moves
                            S_MOVELIST list[1];
                            GenerateAllMoves(pos, list);
                            
                            // Pick the best move using simple evaluation (instant)
                            int engineMove = NOMOVE;
                            int bestScore = -999999;
                            
                            if (list->count > 0) {
                                for (int i = 0; i < list->count; i++) {
                                    int move = list->moves[i].move;
                                    
                                    // Try the move
                                    if (MakeMove(pos, move)) {
                                        // Simple evaluation: just count material
                                        int score = -EvalPosition(pos);
                                        
                                        // Unmake the move
                                        TakeMove(pos);
                                        
                                        // Keep track of best move
                                        if (score > bestScore) {
                                            bestScore = score;
                                            engineMove = move;
                                        }
                                    }
                                }
                            }
                            
                            // Make the engine's move if it found one
                            if (engineMove != NOMOVE) {
                                printf("Engine found move: %s\n", PrMove(engineMove));
                                
                                // Add engine move to history
                                char engineMoveStr[10];
                                snprintf(engineMoveStr, sizeof(engineMoveStr), "%s", PrMove(engineMove));
                                AddMoveToHistory(gui, engineMoveStr);
                                
                                MakeMove(pos, engineMove);
                                printf("✓ Engine played: %s\n", PrMove(engineMove));
                                printf("Side after engine move: %s (%d)\n", pos->side == WHITE ? "WHITE" : "BLACK", pos->side);
                                
                                // Add increment to engine's time
                                if (pos->side == WHITE) {
                                    // Black (engine) just moved
                                    gui->blackTimeMs += gui->incrementMs;
                                } else {
                                    // White (engine) just moved
                                    gui->whiteTimeMs += gui->incrementMs;
                                }
                                gui->lastMoveTime = GetTimeMs();
                                
                                // Check for checkmate/stalemate after engine move
                                if (checkresult(pos) == TRUE) {
                                    SetGameOver(gui, pos);
                                    printf("*** GAME OVER ***\n");
                                    return; // Game has ended
                                }
                            } else {
                                printf("✗ Engine couldn't find a move!\n");
                                // If engine can't find a move, check game state
                                if (checkresult(pos) == TRUE) {
                                    SetGameOver(gui, pos);
                                    printf("*** GAME OVER ***\n");
                                    return; // Game has ended
                                }
                            }
                        }
                    } else {
                        // Player vs Player mode - just switch turns, no engine move
                        printf("PvP Mode: It's now %s's turn\n", pos->side == WHITE ? "WHITE" : "BLACK");
                    }
                } else {
                    printf("✗ MakeMove failed! Move was invalid.\n");
                    gui->selectedSquare = NO_SQ;
                    
                    // If the user can't make any legal moves, check for checkmate/stalemate
                    if (checkresult(pos) == TRUE) {
                        SetGameOver(gui, pos);
                        printf("*** GAME OVER ***\n");
                        return; // Game has ended
                    }
                }
            } else {
                printf("✗ Invalid move: %s\n", moveStr);
                gui->selectedSquare = NO_SQ;
            }
        }
    }
}

void RunGUI(S_BOARD* pos, S_SEARCHINFO* info) {
    GUI gui;
    
    printf("Initializing GUI...\n");
    
    if (!InitGUI(&gui)) {
        printf("Failed to initialize GUI\n");
        return;
    }
    
    printf("GUI initialized successfully. Setting up board...\n");
    
    // Initialize board to starting position
    ParseFen(START_FEN, pos);
    
    // Initialize search info
    info->depth = 6;
    info->quit = FALSE;
    info->stopped = FALSE;
    info->GAME_MODE = CONSOLEMODE; // Set a default mode
    
    printf("Board set to starting position. Starting main loop...\n");
    printf("\n=== GAMBIT CHESS ===\n");
    printf("Current mode: %s\n", gui.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
    printf("Controls:\n");
    printf("  N - New Game\n");
    printf("  M - Switch Mode (PvE/PvP)\n");
    printf("  H - Show help\n");
    printf("=============================\n\n");
    
    SDL_Event e;
    
    while (gui.isRunning) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                gui.isRunning = 0;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    HandleMouseClick(&gui, pos, info, e.button.x, e.button.y);
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                // Scroll move history
                if (e.wheel.y > 0) {
                    // Scroll up
                    gui.historyScrollOffset = (gui.historyScrollOffset > 0) ? gui.historyScrollOffset - 2 : 0;
                } else if (e.wheel.y < 0) {
                    // Scroll down
                    int maxScroll = (gui.moveCount > 20) ? gui.moveCount - 20 : 0;
                    gui.historyScrollOffset = (gui.historyScrollOffset < maxScroll) ? gui.historyScrollOffset + 2 : maxScroll;
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_n) {
                    // New game
                    ParseFen(START_FEN, pos);
                    gui.selectedSquare = NO_SQ;
                    gui.gameOver = 0;
                    strcpy(gui.gameOverMessage, "");
                    
                    // Reset move highlighting
                    gui.possibleMovesCount = 0;
                    
                    // Reset promotion state
                    gui.promotionPending = 0;
                    gui.promotionFromSq = NO_SQ;
                    gui.promotionToSq = NO_SQ;
                    
                    // Reset move history
                    gui.moveCount = 0;
                    gui.historyScrollOffset = 0;
                    for (int i = 0; i < MAX_DISPLAY_MOVES; i++) {
                        strcpy(gui.moveHistory[i], "");
                    }
                    
                    // Reset timers
                    ResetTimers(&gui);
                    
                    printf("New game started in %s mode\n", gui.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
                } else if (e.key.keysym.sym == SDLK_m) {
                    // Switch game mode
                    gui.gameMode = (gui.gameMode == MODE_PVE) ? MODE_PVP : MODE_PVE;
                    printf("Switched to %s mode\n", gui.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
                    printf("Press 'N' for new game to apply mode change\n");
                } else if (e.key.keysym.sym == SDLK_h) {
                    // Show help
                    printf("\n=== CONTROLS ===\n");
                    printf("N - New Game\n");
                    printf("M - Switch Mode (PvE/PvP)\n");
                    printf("H - Show this help\n");
                    printf("Current mode: %s\n", gui.gameMode == MODE_PVE ? "Player vs Engine" : "Player vs Player");
                    printf("================\n\n");
                }
            }
        }
        
        // Update timer
        UpdateTimer(&gui, pos);
        
        RenderBoard(&gui, pos);
        SDL_Delay(16); // ~60 FPS
    }
    
    printf("GUI loop ended. Cleaning up...\n");
    CleanupGUI(&gui);
    printf("GUI cleanup complete.\n");
}