// InputEvent.h - toolkit-agnostic input events for MVC controller
#ifndef SOLID_MVC_INPUTEVENT_H
#define SOLID_MVC_INPUTEVENT_H

#include "types_definitions.h"

enum class InputAction {
    NONE,
    SQUARE_CLICKED,
    NEW_GAME,
    SWITCH_MODE,
    SHOW_HELP,
    QUIT,
    SCROLL_UP,
    SCROLL_DOWN
};

struct InputEvent {
    InputAction action;
    int squareIndex;
    int mouseX;
    int mouseY;

    InputEvent()
        : action(InputAction::NONE), squareIndex(NO_SQ), mouseX(0), mouseY(0) {}
};

// Game modes shared between controller and view (aligned with GUI runtime values).
constexpr int GAME_MODE_PVE = 0;
constexpr int GAME_MODE_PVP = 1;

#endif // SOLID_MVC_INPUTEVENT_H
