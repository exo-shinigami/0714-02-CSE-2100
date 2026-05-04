/**
 * @file gui_input_handler.h
 * @brief GUI Input Handler - Handles all user input (SRP)
 * 
 * This class follows Single Responsibility Principle by focusing only
 * on input handling. It does not render graphics or manage game state.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef GUI_INPUT_HANDLER_H
#define GUI_INPUT_HANDLER_H

#include "../../mvc/InputEvent.h"
#include <SDL2/SDL.h>

/**
 * @class GUIInputHandler
 * @brief Responsible only for handling user input
 * 
 * SOLID Principles Applied:
 * - SRP: Only handles input processing
 * - OCP: Can extend with new input types without modifying existing code
 */
class GUIInputHandler {
private:
    int squareFromCoords(int x, int y) const;
    
public:
    GUIInputHandler() = default;
    ~GUIInputHandler() = default;
    
    // Process SDL events and return input actions
    InputEvent processEvent(const SDL_Event& event) const;
    
    // Check if coordinates are within board area
    bool isInBoardArea(int x, int y) const;
    
    // Get square coordinates for rendering
    void getSquareCoords(int square, int* x, int* y) const;
};

#endif // GUI_INPUT_HANDLER_H
