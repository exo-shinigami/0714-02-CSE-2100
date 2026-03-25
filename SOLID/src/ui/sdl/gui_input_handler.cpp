/**
 * @file gui_input_handler.cpp
 * @brief Implementation of GUIInputHandler class
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#include "gui_input_handler.h"

// Constants from sdl_gui.h
#define BOARD_SIZE 640
#define SQUARE_SIZE 80

int GUIInputHandler::squareFromCoords(int x, int y) const {
    if (!isInBoardArea(x, y)) {
        return NO_SQ;
    }
    
    int file = x / SQUARE_SIZE;
    int rank = 7 - (y / SQUARE_SIZE);
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return NO_SQ;
    }
    
    // Convert to 120-square board representation
    return FILE_RANK_TO_SQUARE(file, rank);
}

bool GUIInputHandler::isInBoardArea(int x, int y) const {
    return (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE);
}

void GUIInputHandler::getSquareCoords(int square, int* x, int* y) const {
    if (x == nullptr || y == nullptr) {
        return;
    }
    
    int file = g_filesBoard[square];
    int rank = g_ranksBoard[square];
    
    *x = file * SQUARE_SIZE;
    *y = (7 - rank) * SQUARE_SIZE;
}

InputEvent GUIInputHandler::processEvent(const SDL_Event& event) const {
    InputEvent result;
    
    if (event.type == SDL_QUIT) {
        result.action = InputAction::QUIT;
        return result;
    }
    
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            int x = event.button.x;
            int y = event.button.y;
            
            result.mouseX = x;
            result.mouseY = y;
            
            if (isInBoardArea(x, y)) {
                result.action = InputAction::SQUARE_CLICKED;
                result.squareIndex = squareFromCoords(x, y);
            }
        }
        return result;
    }
    
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_n:
                result.action = InputAction::NEW_GAME;
                break;
            case SDLK_m:
                result.action = InputAction::SWITCH_MODE;
                break;
            case SDLK_h:
                result.action = InputAction::SHOW_HELP;
                break;
            case SDLK_ESCAPE:
            case SDLK_q:
                result.action = InputAction::QUIT;
                break;
            case SDLK_UP:
                result.action = InputAction::SCROLL_UP;
                break;
            case SDLK_DOWN:
                result.action = InputAction::SCROLL_DOWN;
                break;
            default:
                result.action = InputAction::NONE;
                break;
        }
        return result;
    }
    
    return result;
}
