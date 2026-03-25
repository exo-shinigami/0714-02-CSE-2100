# Gambit Chess

## Description

Gambit Chess is a lightweight yet feature‑rich chess game for Windows, written in C with SDL2. It combines a classic desktop look with modern conveniences: move highlighting, king‑in‑check alerts, full pawn promotion choices, clear captured‑pieces display, and a built‑in engine for solo play. All rules follow real‑world chess, including checkmate, stalemate, threefold repetition, fifty‑move rule, and draws by insufficient material (for example, king vs king).

## Technology Stack

- **Language:** C
- **Graphics:** SDL2
- **Text rendering:** SDL2_ttf
- **Engine architecture:**
	- 10×12 mailbox board representation
	- Bitboards for pawn structure and masks
	- Incremental material tracking
	- Full legal move generation (castling, en passant, promotions)
	- Basic search with move ordering (MVV‑LVA, killers, history)
	- Polyglot opening book support (optional, via `polybook.c` / `polykeys.c`)

## Features

- **Modern board UI**
	- Unicode chess symbols (♔♕♖♗♘♙)
	- Smooth square coloring with board border
	- Side panel showing captured pieces in **two columns** (Black | White)

- **Move assistance**
	- Click a piece to **highlight all legal moves** in green
	- Selected piece square highlighted in yellow
	- King square turns **red** when in check

- **Game rules & endings**
	- Fully legal move generation (castling, en passant, promotion)
	- **Promotion dialog**: choose Queen, Rook, Bishop, or Knight
	- Correct draw handling:
		- Insufficient material (e.g. king vs king)
		- Threefold repetition
		- Fifty‑move rule
		- Stalemate (no legal moves, not in check)
	- Checkmate detection with clear “White/Black Wins” messages

- **Game modes**
	- **Player vs Player** (local)
	- **Player vs Engine** (embedded engine as opponent)

- **Extras**
	- Scrollable move history panel
	- Per‑side timers with highlight on the active player
	- Console help and debug output for analysis

## How to Play

### Starting the Game

JUST DOWNLOAD THE ENTIRE REPOSITORY & PLAY THE GAME BY LAUNCHING "gambit.exe" .
[After downloading ,extract the only zip file in the directory]

### Controls

- **Left‑click piece**: Select / deselect a piece.
	- All legal moves for that piece highlight in **green**.
	- Click a highlighted square to move.
- **Timers**: The active side’s clock is outlined.
- **Captured pieces**: View material balance in the right panel.
- **Keyboard shortcuts:**
	- `N` – New game
	- `M` – Switch mode (Player vs Player / Player vs Engine)
	- `H` – Print help in console

### Pawn Promotion

When a pawn reaches the back rank:

1. A promotion dialog appears in the center of the window.
2. Click one of the four piece options:
	 - Queen, Rook, Bishop, or Knight (no King).
3. The pawn is replaced by the chosen piece and the game continues.

## Setup & Build

### Prerequisites

- **OS:** Windows 7 or later
- **Compiler:** MinGW‑w64 (or another GCC‑compatible toolchain)
- **Libraries:**
	- SDL2
	- SDL2_ttf

Ensure SDL2 and SDL2_ttf development libraries are installed and available to your compiler.

### Building from Source (MinGW / make)

From the `src` directory (where `makefile` lives):

```sh
make
```

This will:

- Compile all `.c` files into `gambit.exe`
- Attempt to copy `SDL2.dll` and `SDL2_ttf.dll` into the build directory (if found)

If DLLs are not found automatically, copy them manually next to `gambit.exe`.

### Running

From the same directory as `gambit.exe` and the DLLs:

```sh
./gambit.exe
```

or just double‑click `gambit.exe` in Explorer.

## Repository Structure (Key Files)

- `gambit.c` – Main entry point
- `gui.c`, `gui.h` – SDL2 graphical interface (board, panels, timers, input)
- `board.c`, `data.c`, `defs.h` – Core board representation and constants
- `movegen.c` – Move generation (legal moves, captures, promotions)
- `makemove.c` – Making/unmaking moves, maintaining game state
- `search.c`, `evaluate.c` – Engine search and evaluation
- `xboard.c`, `uci.c` – Protocol support (text‑mode / engine integration)
- `polybook.c`, `polykeys.c` – Polyglot opening book support
- `PORTABLE_PACKAGE.txt` – Notes on portable distribution

## License

This project is open source. Feel free to modify and distribute.
