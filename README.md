# Gambit Chess Engine Handbook

Gambit is a GUI-only chess application written in C++ and organized around an MVC architecture.
This repository is intended to be distributed as a ready-to-run Windows package: a user should be able to extract the archive and launch the game without compiling the project.
In this repository layout, the runnable application lives in the `MVC/` subfolder.

## Overview

Gambit provides a classic desktop chess experience with a board view, move history, captured-piece tracking, timers, promotion handling, and a controller-driven game loop.
The shipped release is intentionally GUI-only. Console protocol modes are not part of the customer-facing runtime.

The application is built around three responsibilities:

- Model: chess state, move execution, and rules.
- View: SDL-based rendering and input polling.
- Controller: game flow, selection handling, move application, and promotion decisions.

## What A User Should Do

If you downloaded the repository ZIP from GitHub, extract it and open the project folder that contains the executable and the `lib/` directory.
In this repository, that folder is `MVC/`.
From there, launch `gambit.exe` by double-clicking it.

You do not need to build the project first.

## First Launch Checklist

1. Extract the ZIP file fully.
2. Keep `gambit.exe` and the bundled `lib/` directory together.
3. Double-click `gambit.exe`.
4. Wait for the chess window to appear.
5. Start playing by clicking one piece and then its destination square.

If the executable is moved away from its bundled DLLs, SDL may not load correctly.

## Controls

### Mouse

- Left click a piece to select it.
- Left click a destination square to move the selected piece.

### Keyboard

- `N` - Start a new game.
- `M` - Toggle Player vs Player and Player vs Engine mode.
- `H` - Show in-app help text.
- `Esc` or `Q` - Quit the application.
- `Arrow Up` / `Arrow Down` - Scroll through move history.

### Promotion

When a pawn reaches the final rank, the promotion flow opens automatically.
The controller handles the promotion choice and applies the final move through the model.

## Game Modes

The game starts in Player vs Player mode by default.
Press `M` if you want to switch to Player vs Engine.

In Player vs Engine mode, the built-in move policy picks the engine response after your move.

## On-Screen Layout

The GUI is organized into three visible areas:

- Main board area for chess play.
- Side panel for captured pieces.
- Side panel for move history and game state information.

The window also shows timer information and status messages when applicable.

## Distribution Contents

The repository is structured so the runnable package can be distributed as a ZIP without a build step.

Important shipped files include:

- `MVC/gambit.exe` - the executable.
- `MVC/lib/SDL2.dll` - SDL runtime library.
- `MVC/lib/SDL2_ttf.dll` - SDL_ttf runtime library.

Keep those files together in the same extracted folder.

## System Requirements

Recommended runtime environment:

- 64-bit Windows 10 or newer.
- A standard desktop session with normal graphics support.
- The bundled SDL runtime DLLs present beside the executable.

For most modern Windows 10/11 systems, the shipped package should start without any manual build step.
On heavily stripped or very old installations, a modern Microsoft runtime may also be required.

## Project Structure

| Path | Purpose |
| --- | --- |
| `MVC/src/main/` | Application entry point and startup wiring. |
| `MVC/src/mvc/` | MVC interfaces, controller, model adapter, and SDL-backed view. |
| `MVC/src/core/` | Board representation, move generation, move execution, validation, and low-level chess rules. |
| `MVC/src/engine/` | Search, evaluation, and hash-table code. |
| `MVC/src/ui/sdl/` | SDL GUI composition, input translation, timers, history tracking, and promotion helpers. |
| `MVC/lib/` | Bundled runtime DLLs required by the Windows release. |
| `MVC/build/` | Generated object files and build output. |
| `MVC/scripts/` | Packaging and setup scripts. |
| `MVC/docs/` | Supplemental technical documentation. |

## How The MVC Flow Works

The runtime flow is intentionally simple:

1. SDL receives a mouse or keyboard event.
2. The view converts it into a toolkit-agnostic `InputEvent`.
3. The controller interprets the event and decides what game action should happen.
4. The model applies the move or updates the board state.
5. The view renders the updated board, timers, and game state.

This separation keeps platform-specific code in the view and game logic in the controller and model.

## Build From Source

End users do not need this section.
It is included for maintainers and developers who want to rebuild the project.

### Windows Development Environment

Use MSYS2 / MinGW UCRT64 or an equivalent Windows C++ toolchain with SDL2 and SDL2_ttf installed.

Typical build command from inside the `MVC/` folder:

```bash
mingw32-make all
```

The makefile also accepts `make gui` as a compatibility alias.

### Linux Development Environment

Install a recent C++ compiler and the SDL development packages, then build from the `MVC/` folder:

```bash
make all
```

### Build Output

The build produces:

- `MVC/build/bin/gambit.exe`
- a copied executable at `MVC/gambit.exe`

## Important Source Files

| File | Role |
| --- | --- |
| `MVC/src/main/main_entry.cpp` | Program entry point and startup wiring. |
| `MVC/src/mvc/controllers/ControllerImpl.cpp` | Coordinates input, move application, and game flow. |
| `MVC/src/mvc/views/SDLView.cpp` | SDL-backed view implementation. |
| `MVC/src/mvc/adapters/CoreModelAdapter.cpp` | Wraps the chess engine core behind the model interface. |
| `MVC/src/core/moves/moves_generation.cpp` | Generates legal and pseudo-legal chess moves. |
| `MVC/src/core/moves/moves_execution.cpp` | Applies and retracts moves on the board. |
| `MVC/src/ui/sdl/sdl_gui.cpp` | SDL window, rendering, fonts, and UI composition. |
| `MVC/makefile` | Build rules and platform-specific link flags. |

## Troubleshooting

### The game does not start

- Confirm that `gambit.exe` is still next to the `lib/` directory.
- Make sure the ZIP was fully extracted.
- Check whether security software quarantined one of the bundled DLLs.

### The window opens but input feels broken

- Make sure the chess window is focused before clicking.
- Use a left click on a piece first, then click the destination square.
- Press `N` to reset the board if you want to start over.

### Windows reports missing runtime components

- Recheck that the bundled SDL DLLs are present.
- On older or stripped Windows systems, install the current Microsoft runtime package used by your system.

### I want a fresh start from source

- Clean the tree with the makefile `clean` target.
- Rebuild with `mingw32-make all` on Windows or `make all` on Linux.

## Design Notes

The project is structured to keep responsibilities separated:

- The model owns chess rules and state.
- The view owns SDL and rendering details.
- The controller owns user interaction and the game loop.

That design keeps the codebase easier to test, easier to maintain, and safer to extend.

## Launch Options

The only supported command-line flag in the shipped GUI release is:

```bash
gambit.exe NoBook
```

This disables the opening book during startup.

## Summary

This repository is intended to be a polished, direct-play Windows chess game.
For customers, the expected experience is simple: extract the ZIP, open the folder that contains the executable, and start playing.
For developers, the source tree remains organized so the MVC boundary stays clear and the project can still be rebuilt cleanly when needed.
