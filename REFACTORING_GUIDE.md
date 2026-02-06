# Gambit Chess - Code Refactoring & Software Engineering Standards Guide

**Course:** Advanced Programming Lab - 2nd Year CSE  
**Project:** Gambit Chess Engine  
**Purpose:** Establish consistent, professional, and modular coding standards  
**Date:** February 2026

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [New Naming Conventions](#new-naming-conventions)
3. [New Naming Style](#new-naming-style)
4. [New Folder Structure](#new-folder-structure)
5. [Programming Style Guidelines](#programming-style-guidelines)
6. [SOLID Principles Implementation](#solid-principles-implementation)
7. [Design Patterns to Apply](#design-patterns-to-apply)
8. [Migration Roadmap](#migration-roadmap)
9. [Before & After Examples](#before--after-examples)

---

## Executive Summary

### Current State Analysis

The Gambit Chess engine is a functional C-based chess application with the following characteristics:

**Strengths:**
- Working chess engine with full rule implementation
- Clear separation of concerns (board, moves, search, UI)
- Efficient data structures (bitboards, 10×12 mailbox)

**Areas for Improvement:**
- Inconsistent naming (mix of Hungarian notation, abbreviations, and full names)
- Flat file structure (all source files in root directory)
- Global variables scattered throughout
- Tight coupling between modules
- Limited abstraction and encapsulation
- Hard to test individual components

### Refactoring Philosophy

> "We're not rewriting from scratch—we're systematically improving what works, making it cleaner, more maintainable, and easier to extend."

This guide focuses on **evolutionary refactoring**: improving the codebase incrementally while maintaining functionality at each step.

---

## New Naming Conventions

### General Principles

1. **Be Descriptive, Not Cryptic**: Names should reveal intent
2. **Consistency Over Brevity**: Prefer `capturedPieceCount` over `capPceCnt`
3. **Avoid Abbreviations**: Unless universally understood (e.g., `id`, `max`, `min`)
4. **Use Domain Language**: Chess-specific terms should be clear

### Variable Naming

#### Current Problems
```c
int sq64, sq120;           // What do these numbers mean?
int t_pceNum[13];          // What's 't_'? What's 'pce'?
S_BOARD *pos;              // Why 'pos'? Position? Pointer to Struct?
int pceNum[13];            // Inconsistent with piece naming elsewhere
```

#### New Standards

**Local Variables:**
- Use `camelCase` for local variables
- Descriptive names for important variables
- Single letters only for trivial loop counters

```c
// Before
int sq64, sq120, t_piece, t_pce_num;

// After
int square64Index, square120Index;
int temporaryPieceType, temporaryPieceCount;
```

**Constants:**
- Use `UPPER_SNAKE_CASE` for constants
- Prefix with module name if scope is global

```c
// Before
#define MAXGAMEMOVES 2048
#define START_FEN "rnbqkbnr/..."

// After
#define CHESS_MAX_GAME_MOVES 2048
#define CHESS_STARTING_POSITION_FEN "rnbqkbnr/..."
```

**Global Variables:**
- Prefix with `g_` to indicate global scope
- Use `camelCase` after prefix

```c
// Before
extern int Sq120ToSq64[BRD_SQ_NUM];
extern U64 SetMask[64];

// After
extern int g_square120To64[BOARD_SQUARE_COUNT];
extern U64 g_bitSetMask[64];
```

### Function Naming

#### Current Problems
```c
void AllInit();                    // Too vague
int ParseFen(char *fen, ...);      // Inconsistent capitalization
int SqAttacked(...);               // Abbreviated
void GenerateAllMoves(...);        // Inconsistent verb form
```

#### New Standards

**Naming Pattern:**
- Use `PascalCase` for function names (C convention for public APIs)
- Use verb-noun structure
- Module prefix for public functions

```c
// Before
void AllInit();
int ParseFen(char *fen, S_BOARD *pos);
int SqAttacked(const int sq, ...);
void GenerateAllMoves(...);

// After
void Chess_InitializeEngine(void);
int Board_ParseFromFEN(const char *fenString, ChessBoard *board);
bool Square_IsUnderAttack(SquareIndex square, PieceColor attackerColor, const ChessBoard *board);
void MoveGen_GenerateAllLegalMoves(const ChessBoard *board, MoveList *moveList);
```

**Function Name Components:**
1. **Module prefix**: `Board_`, `Move_`, `Search_`, etc.
2. **Action verb**: `Initialize`, `Parse`, `Generate`, `Calculate`, etc.
3. **Object**: What is being acted upon
4. **Qualifier**: Additional context if needed

### Structure and Type Naming

#### Current Problems
```c
typedef struct {
    int move;
    int score;
} S_MOVE;                    // Hungarian notation prefix

typedef struct {
    U64 posKey;
    int move;
} S_HASHENTRY;               // Unclear purpose
```

#### New Standards

**Type Definitions:**
- Use `PascalCase` without prefixes
- Descriptive names that reveal purpose
- Typedef pointer types separately if commonly used

```c
// Before
typedef struct {
    int move;
    int score;
} S_MOVE;

typedef struct {
    S_MOVE moves[MAXPOSITIONMOVES];
    int count;
} S_MOVELIST;

// After
typedef struct {
    Move move;
    int evaluationScore;
} ScoredMove;

typedef struct {
    ScoredMove moves[CHESS_MAX_POSITION_MOVES];
    int moveCount;
} MoveList;

// Common pointer types
typedef ChessBoard* ChessBoardPtr;
typedef MoveList* MoveListPtr;
```

### Enumeration Naming

#### Current Problems
```c
enum { EMPTY, wP, wN, wB, ...};    // Inconsistent, unclear
enum { FILE_A, FILE_B, ...};       // Good!
enum { WHITE, BLACK, BOTH };       // Mixing concerns
```

#### New Standards

**Enum Type Names:**
- Use `PascalCase` with descriptive name
- Consider typedef for cleaner usage

**Enum Values:**
- Prefix with abbreviated type name
- Use `UPPER_SNAKE_CASE`

```c
// Before
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { WHITE, BLACK, BOTH };

// After
typedef enum {
    PIECE_TYPE_NONE = 0,
    PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING
} PieceType;

typedef enum {
    PIECE_COLOR_WHITE = 0,
    PIECE_COLOR_BLACK,
    PIECE_COLOR_BOTH  // For bitboard operations
} PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

// Now pieces are represented as:
// Piece whitePawn = { PIECE_TYPE_PAWN, PIECE_COLOR_WHITE };
```

### Macro Naming

#### Current Problems
```c
#define FROMSQ(m) ((m) & 0x7F)      // Abbreviated
#define TOSQ(m) (((m)>>7) & 0x7F)   // Unclear bit manipulation
#define CLRBIT(bb,sq) ...           // Abbreviated
```

#### New Standards

**Macro Names:**
- Use `UPPER_SNAKE_CASE`
- Descriptive action verbs
- Type prefixes for related macros

```c
// Before
#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])

// After
#define MOVE_GET_FROM_SQUARE(move) ((move) & 0x7F)
#define MOVE_GET_TO_SQUARE(move) (((move) >> 7) & 0x7F)
#define MOVE_GET_CAPTURED_PIECE(move) (((move) >> 14) & 0xF)
#define BITBOARD_CLEAR_BIT(bitboard, square) ((bitboard) &= g_bitClearMask[(square)])
#define BITBOARD_SET_BIT(bitboard, square) ((bitboard) |= g_bitSetMask[(square)])
```

---

## New Naming Style

### File Naming Conventions

#### Current State
```
gambit.c          (main entry)
board.c           (mixed functionality)
defs.h            (everything header)
gui.c             (UI)
```

#### New Standards

**Pattern:** `module_component.c` / `module_component.h`

```
chess_engine.c          (main entry point)
chess_types.h           (common type definitions)
chess_constants.h       (global constants)

board_representation.c
board_representation.h
board_initialization.c
board_validation.c

move_generation.c
move_generation.h
move_validation.c
move_execution.c

search_algorithm.c
search_evaluation.c
search_hashtable.c

ui_console.c
ui_sdl_graphics.c
ui_sdl_input.c
```

### Code Organization Style

#### Module Headers

Every module should start with a standardized header:

```c
/**
 * @file board_representation.c
 * @brief Core board representation and state management
 * 
 * This module handles the internal representation of the chess board
 * including piece placement, castling rights, en passant, and move history.
 * 
 * @author [Your Name]
 * @date February 2026
 * @version 2.0
 */

#include "board_representation.h"
#include "chess_types.h"
#include "chess_constants.h"
```

#### Function Documentation Style

```c
/**
 * @brief Checks if a square is under attack by a specific color
 * 
 * This function determines whether a given square on the chess board
 * is currently attacked by any piece of the specified color. Used for
 * check detection, castling validation, and move legality.
 * 
 * @param square The square to check (0-63 or 21-98 in 120-square representation)
 * @param attackerColor The color of the attacking pieces (WHITE or BLACK)
 * @param board Pointer to the current board state
 * 
 * @return true if the square is under attack, false otherwise
 * 
 * @note This function does not modify the board state
 * @complexity O(1) average case, uses precomputed attack tables
 */
bool Square_IsUnderAttack(
    SquareIndex square,
    PieceColor attackerColor,
    const ChessBoard *board
);
```

#### Comment Style

**Inline Comments:**
- Use `//` for single-line comments
- Place above the code, not at the end of lines (unless very short)
- Explain "why", not "what"

```c
// BAD: What the code does (obvious)
int score = 0;  // Initialize score to 0

// GOOD: Why we're doing it
int score = 0;  // Material evaluation starts at equal position
```

**Block Comments:**
- Use `/* */` for multi-line algorithm explanations
- Include before complex logic

```c
/*
 * Principal Variation Search (PVS) Implementation
 * ------------------------------------------------
 * We search the first move with a full window [alpha, beta].
 * Subsequent moves use a null window [alpha, alpha+1] for faster
 * search. If a move fails high, we re-search with full window.
 * This reduces the average number of nodes searched.
 */
int pvScore = Search_PrincipalVariation(board, depth, alpha, beta);
```

---

## New Folder Structure

### Current Structure Problems

```
gambit-chess-main/
├── gambit.c
├── board.c
├── attack.c
├── bitboards.c
├── data.c
├── defs.h
├── gui.c
├── gui.h
├── search.c
├── evaluate.c
├── movegen.c
├── makemove.c
└── (30+ files in root)
```

**Issues:**
- Flat structure makes navigation difficult
- No logical grouping of related files
- Headers mixed with implementation
- Build artifacts mixed with source

### Proposed Structure

```
gambit-chess/
│
├── docs/                           # Documentation
│   ├── README.md                  # Project overview
│   ├── USER_GUIDE.md              # How to play
│   ├── ARCHITECTURE.md            # System design
│   ├── API_REFERENCE.md           # Code documentation
│   └── REFACTORING_GUIDE.md       # This document
│
├── src/                           # All source code
│   ├── main/                      # Main entry point
│   │   └── chess_engine.c        # Main function
│   │
│   ├── core/                      # Core chess logic (no UI dependencies)
│   │   ├── types/                # Type definitions
│   │   │   ├── chess_types.h
│   │   │   ├── chess_constants.h
│   │   │   └── chess_macros.h
│   │   │
│   │   ├── board/                # Board representation
│   │   │   ├── board_representation.h
│   │   │   ├── board_representation.c
│   │   │   ├── board_initialization.c
│   │   │   ├── board_validation.c
│   │   │   └── board_fen_parser.c
│   │   │
│   │   ├── moves/                # Move generation and execution
│   │   │   ├── move_generation.h
│   │   │   ├── move_generation.c
│   │   │   ├── move_generation_pawn.c
│   │   │   ├── move_generation_knight.c
│   │   │   ├── move_generation_sliding.c
│   │   │   ├── move_validation.c
│   │   │   ├── move_execution.c
│   │   │   └── move_ordering.c
│   │   │
│   │   ├── bitboards/            # Bitboard operations
│   │   │   ├── bitboard_operations.h
│   │   │   ├── bitboard_operations.c
│   │   │   └── bitboard_masks.c
│   │   │
│   │   └── attack/               # Attack detection
│   │       ├── attack_detection.h
│   │       └── attack_detection.c
│   │
│   ├── engine/                    # Chess engine (AI)
│   │   ├── search/               # Search algorithms
│   │   │   ├── search_algorithm.h
│   │   │   ├── search_algorithm.c
│   │   │   ├── search_alphabeta.c
│   │   │   ├── search_quiescence.c
│   │   │   └── search_time_manager.c
│   │   │
│   │   ├── evaluation/           # Position evaluation
│   │   │   ├── evaluation.h
│   │   │   ├── evaluation.c
│   │   │   ├── evaluation_material.c
│   │   │   ├── evaluation_pawn_structure.c
│   │   │   └── evaluation_piece_square_tables.c
│   │   │
│   │   └── hashtable/            # Transposition table
│   │       ├── hashtable.h
│   │       ├── hashtable.c
│   │       └── zobrist_hashing.c
│   │
│   ├── ui/                        # User interface layer
│   │   ├── console/              # Console UI
│   │   │   ├── ui_console.h
│   │   │   ├── ui_console.c
│   │   │   └── ui_console_input.c
│   │   │
│   │   ├── sdl/                  # SDL2 GUI
│   │   │   ├── ui_sdl.h
│   │   │   ├── ui_sdl_main.c
│   │   │   ├── ui_sdl_rendering.c
│   │   │   ├── ui_sdl_input.c
│   │   │   └── ui_sdl_dialogs.c
│   │   │
│   │   └── protocols/            # UCI and XBoard
│   │       ├── protocol_uci.h
│   │       ├── protocol_uci.c
│   │       ├── protocol_xboard.h
│   │       └── protocol_xboard.c
│   │
│   ├── utils/                     # Utility functions
│   │   ├── utils_time.h
│   │   ├── utils_time.c
│   │   ├── utils_io.h
│   │   ├── utils_io.c
│   │   └── utils_string.c
│   │
│   └── openingbook/              # Opening book support
│       ├── polyglot_book.h
│       ├── polyglot_book.c
│       └── polyglot_keys.c
│
├── include/                       # Public header files (if creating library)
│   └── gambit_chess.h            # Single public API header
│
├── tests/                         # Unit tests
│   ├── test_board.c
│   ├── test_moves.c
│   ├── test_search.c
│   └── test_evaluation.c
│
├── build/                         # Build output (gitignored)
│   ├── obj/                      # Object files
│   └── bin/                      # Executables
│
├── assets/                        # Game assets
│   ├── fonts/                    # TTF fonts
│   └── config/                   # Configuration files
│
├── scripts/                       # Build and utility scripts
│   ├── build.sh
│   ├── build.bat
│   ├── run_tests.sh
│   └── clean.sh
│
├── lib/                          # External libraries (SDL2 DLLs)
│   ├── SDL2.dll
│   └── SDL2_ttf.dll
│
├── Makefile                      # Build configuration
├── .gitignore                    # Git ignore file
├── LICENSE                       # License file
└── CHANGELOG.md                  # Version history
```

### Folder Organization Principles

1. **Separation of Concerns**: Core logic independent of UI
2. **Module Cohesion**: Related files grouped together
3. **Clear Dependencies**: Dependencies flow downward (UI → Engine → Core)
4. **Testability**: Core modules can be tested without UI
5. **Build Separation**: Source separate from build artifacts

---

## Programming Style Guidelines

### General Code Style

#### Indentation and Spacing

```c
// Use 4 spaces for indentation (no tabs)
// Place opening braces on same line for functions
// Use consistent spacing around operators

// Before
if(x==5){
int y=10;
return y;}

// After
if (x == 5) {
    int y = 10;
    return y;
}
```

#### Line Length
- Maximum 100 characters per line
- Break long function calls logically

```c
// Before
int result = CalculateComplexEvaluation(board, depth, alpha, beta, timeLimit, maxNodes);

// After
int result = CalculateComplexEvaluation(
    board,
    depth,
    alpha,
    beta,
    timeLimit,
    maxNodes
);
```

### Function Guidelines

#### Function Length
- Keep functions under 50 lines
- Extract complex logic into helper functions
- One function = one responsibility

```c
// Before: Long monolithic function
void ProcessMove(S_BOARD *pos, int move) {
    // 200 lines of mixed validation, execution, and UI updates
}

// After: Decomposed into logical units
bool Move_IsValid(const ChessBoard *board, Move move);
void Move_Execute(ChessBoard *board, Move move);
void Move_UpdateUI(const ChessBoard *board, Move move);
void Board_RecordInHistory(ChessBoard *board, Move move);

void Game_ProcessMove(ChessBoard *board, Move move) {
    if (!Move_IsValid(board, move)) {
        return;
    }
    
    Move_Execute(board, move);
    Board_RecordInHistory(board, move);
    Move_UpdateUI(board, move);
}
```

#### Function Parameters
- Maximum 5 parameters
- Use structures for related parameters
- Const-correctness for read-only parameters

```c
// Before: Too many parameters
int EvaluatePosition(S_BOARD *pos, int depth, int alpha, int beta, int color, int castleRights, int enPassant);

// After: Grouped into context structure
typedef struct {
    int depth;
    int alpha;
    int beta;
    int timeLimit;
} SearchContext;

int Evaluation_EvaluatePosition(
    const ChessBoard *board,
    const SearchContext *context
);
```

### Error Handling

#### Return Values and Status Codes

```c
// Define clear status codes
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_INVALID_MOVE,
    STATUS_ERROR_INVALID_FEN,
    STATUS_ERROR_OUT_OF_MEMORY,
    STATUS_ERROR_INVALID_BOARD_STATE
} StatusCode;

// Use return values consistently
StatusCode Board_ParseFromFEN(const char *fenString, ChessBoard *board) {
    if (fenString == NULL || board == NULL) {
        return STATUS_ERROR_INVALID_FEN;
    }
    
    // Parsing logic...
    
    if (!Board_Validate(board)) {
        return STATUS_ERROR_INVALID_BOARD_STATE;
    }
    
    return STATUS_SUCCESS;
}

// Caller checks return value
StatusCode status = Board_ParseFromFEN(fen, &board);
if (status != STATUS_SUCCESS) {
    fprintf(stderr, "Error: Failed to parse FEN string (code: %d)\n", status);
    return status;
}
```

#### Assertions for Programming Errors

```c
// Use assertions for "should never happen" conditions
void Move_Execute(ChessBoard *board, Move move) {
    // This is a programming error if violated
    assert(board != NULL);
    assert(Move_IsValid(board, move));
    
    // Normal error handling for runtime conditions
    if (board->pieces[move.toSquare] != PIECE_TYPE_NONE) {
        // Handle capture
    }
}
```

### Memory Management

#### Allocation Patterns

```c
// Check all allocations
MoveList* MoveList_Create(void) {
    MoveList *list = (MoveList*)malloc(sizeof(MoveList));
    if (list == NULL) {
        fprintf(stderr, "Error: Failed to allocate MoveList\n");
        return NULL;
    }
    
    list->moveCount = 0;
    list->capacity = CHESS_MAX_POSITION_MOVES;
    return list;
}

// Corresponding cleanup
void MoveList_Destroy(MoveList *list) {
    if (list != NULL) {
        free(list);
    }
}
```

#### Ownership and Lifetime

```c
// Document ownership clearly
/**
 * @brief Creates a new chess board
 * @return Pointer to newly allocated board. Caller owns and must call Board_Destroy()
 */
ChessBoard* Board_Create(void);

/**
 * @brief Destroys a chess board and frees memory
 * @param board Board to destroy. Pointer is invalid after this call.
 */
void Board_Destroy(ChessBoard *board);
```

---

## SOLID Principles Implementation

### S - Single Responsibility Principle

**Principle:** Each module/function should have one reason to change.

#### Current Violations

```c
// board.c does too much:
void UpdateListsMaterial(S_BOARD *pos) {
    // Updates piece lists
    // Calculates material count
    // Updates bitboards
    // Validates board state
}
```

#### Refactored Approach

```c
// Separate responsibilities into focused modules

// board_piece_list.c
void PieceList_Update(ChessBoard *board);

// board_material.c
void Material_Calculate(ChessBoard *board);

// board_bitboards.c
void Bitboards_Update(ChessBoard *board);

// board_validation.c
bool Board_Validate(const ChessBoard *board);

// Higher level coordination
void Board_UpdateDerivedState(ChessBoard *board) {
    PieceList_Update(board);
    Material_Calculate(board);
    Bitboards_Update(board);
}
```

### O - Open/Closed Principle

**Principle:** Open for extension, closed for modification.

#### Strategy Pattern for Search Algorithms

```c
// Define interface for search strategies
typedef struct {
    int (*search)(ChessBoard *board, int depth, int alpha, int beta);
    const char *name;
} SearchStrategy;

// Implement different strategies
static int alphaBetaSearch(ChessBoard *board, int depth, int alpha, int beta);
static int principalVariationSearch(ChessBoard *board, int depth, int alpha, int beta);
static int nullMoveSearch(ChessBoard *board, int depth, int alpha, int beta);

// Strategy registry
SearchStrategy g_searchStrategies[] = {
    { alphaBetaSearch, "Alpha-Beta" },
    { principalVariationSearch, "PVS" },
    { nullMoveSearch, "Null-Move" }
};

// Use strategy without modifying existing code
int Search_ExecuteWithStrategy(
    ChessBoard *board,
    int strategyIndex,
    int depth,
    int alpha,
    int beta
) {
    return g_searchStrategies[strategyIndex].search(board, depth, alpha, beta);
}
```

#### Template Pattern for UI Backends

```c
// Abstract UI interface
typedef struct UIBackend {
    void (*initialize)(void);
    void (*render)(const ChessBoard *board);
    Move (*getUserMove)(void);
    void (*cleanup)(void);
} UIBackend;

// Console implementation
static UIBackend consoleUI = {
    .initialize = Console_Initialize,
    .render = Console_Render,
    .getUserMove = Console_GetUserMove,
    .cleanup = Console_Cleanup
};

// SDL implementation
static UIBackend sdlUI = {
    .initialize = SDL_Initialize,
    .render = SDL_Render,
    .getUserMove = SDL_GetUserMove,
    .cleanup = SDL_Cleanup
};

// Client code doesn't change when adding new UI
void Game_Run(UIBackend *ui) {
    ui->initialize();
    
    while (gameIsRunning) {
        ui->render(board);
        Move move = ui->getUserMove();
        // ...
    }
    
    ui->cleanup();
}
```

### L - Liskov Substitution Principle

**Principle:** Subtypes must be substitutable for their base types.

#### Consistent Evaluation Interface

```c
// Base evaluation function signature
typedef int (*EvaluationFunction)(const ChessBoard *board);

// All evaluation functions follow same contract
int Evaluation_MaterialOnly(const ChessBoard *board);
int Evaluation_PositionalSimple(const ChessBoard *board);
int Evaluation_FullEvaluation(const ChessBoard *board);

// Can be used interchangeably
void Search_SetEvaluationFunction(EvaluationFunction evalFunc) {
    g_currentEvaluator = evalFunc;
}

// Usage remains same regardless of evaluator
int score = g_currentEvaluator(board);
```

### I - Interface Segregation Principle

**Principle:** Clients shouldn't depend on interfaces they don't use.

#### Separate Interfaces for Different Clients

```c
// Instead of one large ChessEngine interface:

// Interface for game controllers (minimal)
typedef struct {
    StatusCode (*makeMove)(ChessBoard *board, Move move);
    bool (*isGameOver)(const ChessBoard *board);
    MoveList* (*getLegalMoves)(const ChessBoard *board);
} GameController;

// Interface for AI engine (extensive)
typedef struct {
    Move (*searchBestMove)(const ChessBoard *board, int depth);
    int (*evaluatePosition)(const ChessBoard *board);
    void (*setHashTableSize)(int megabytes);
    void (*clearHashTable)(void);
} AIEngine;

// Interface for UI (display-focused)
typedef struct {
    const char* (*getPieceSymbol)(Piece piece);
    bool (*isSquareAttacked)(const ChessBoard *board, SquareIndex square);
    void (*getSquareColor)(SquareIndex square, Color *color);
} UIHelper;
```

### D - Dependency Inversion Principle

**Principle:** Depend on abstractions, not concretions.

#### Before: Direct Dependencies

```c
// High-level module depends on low-level SDL
void Game_UpdateDisplay(S_BOARD *pos) {
    SDL_RenderClear(renderer);         // Direct SDL dependency
    DrawBoard(pos);
    SDL_RenderPresent(renderer);
}
```

#### After: Dependency Injection

```c
// Abstract rendering interface
typedef struct {
    void (*clearScreen)(void);
    void (*drawBoard)(const ChessBoard *board);
    void (*present)(void);
} Renderer;

// High-level module depends on abstraction
void Game_UpdateDisplay(const ChessBoard *board, Renderer *renderer) {
    renderer->clearScreen();
    renderer->drawBoard(board);
    renderer->present();
}

// SDL implementation
static void SDL_ClearScreen(void) { SDL_RenderClear(g_renderer); }
static void SDL_DrawBoard(const ChessBoard *board) { /* impl */ }
static void SDL_Present(void) { SDL_RenderPresent(g_renderer); }

static Renderer sdlRenderer = {
    .clearScreen = SDL_ClearScreen,
    .drawBoard = SDL_DrawBoard,
    .present = SDL_Present
};

// Console implementation
static Renderer consoleRenderer = { /* ... */ };

// Inject dependency
Game_UpdateDisplay(board, &sdlRenderer);
```

---

## Design Patterns to Apply

### 1. Factory Pattern - Board Creation

```c
/**
 * Factory for creating chess boards in different states
 */

// Board factory interface
typedef enum {
    BOARD_START_POSITION,
    BOARD_EMPTY,
    BOARD_FROM_FEN,
    BOARD_CUSTOM
} BoardCreationType;

ChessBoard* BoardFactory_Create(BoardCreationType type, void *params) {
    ChessBoard *board = Board_Allocate();
    if (board == NULL) {
        return NULL;
    }
    
    switch (type) {
        case BOARD_START_POSITION:
            Board_SetupStartPosition(board);
            break;
            
        case BOARD_EMPTY:
            Board_SetupEmpty(board);
            break;
            
        case BOARD_FROM_FEN:
            if (params == NULL) {
                Board_Destroy(board);
                return NULL;
            }
            Board_ParseFromFEN((const char*)params, board);
            break;
            
        case BOARD_CUSTOM:
            // Custom setup callback
            if (params != NULL) {
                ((BoardSetupCallback)params)(board);
            }
            break;
    }
    
    Board_UpdateDerivedState(board);
    return board;
}

// Usage
ChessBoard *startBoard = BoardFactory_Create(BOARD_START_POSITION, NULL);
ChessBoard *fenBoard = BoardFactory_Create(BOARD_FROM_FEN, "rnbqkbnr/pppppppp/...");
```

### 2. Strategy Pattern - Move Ordering

```c
/**
 * Different move ordering strategies for search optimization
 */

typedef struct {
    void (*orderMoves)(MoveList *moveList, const ChessBoard *board);
    const char *name;
} MoveOrderingStrategy;

// MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
static void orderByMVVLVA(MoveList *moveList, const ChessBoard *board) {
    for (int i = 0; i < moveList->moveCount; i++) {
        Move move = moveList->moves[i].move;
        int victim = board->pieces[MOVE_GET_TO_SQUARE(move)].type;
        int attacker = board->pieces[MOVE_GET_FROM_SQUARE(move)].type;
        moveList->moves[i].score = g_mvvlvaScores[victim][attacker];
    }
    MoveList_Sort(moveList);
}

// History heuristic
static void orderByHistory(MoveList *moveList, const ChessBoard *board) {
    for (int i = 0; i < moveList->moveCount; i++) {
        Move move = moveList->moves[i].move;
        moveList->moves[i].score = g_historyTable[move.fromSquare][move.toSquare];
    }
    MoveList_Sort(moveList);
}

// Strategy registry
MoveOrderingStrategy g_orderingStrategies[] = {
    { orderByMVVLVA, "MVV-LVA" },
    { orderByHistory, "History" },
    { orderByKillers, "Killer Moves" }
};

// Apply strategy
void MoveOrdering_Apply(MoveList *moveList, const ChessBoard *board, int strategyId) {
    g_orderingStrategies[strategyId].orderMoves(moveList, board);
}
```

### 3. Observer Pattern - Game State Changes

```c
/**
 * Notify interested parties when game state changes
 */

typedef enum {
    EVENT_MOVE_MADE,
    EVENT_GAME_OVER,
    EVENT_CHECK,
    EVENT_BOARD_RESET
} GameEvent;

typedef struct GameObserver {
    void (*onGameEvent)(GameEvent event, const ChessBoard *board);
    struct GameObserver *next;
} GameObserver;

typedef struct {
    GameObserver *observers;
} GameSubject;

// Register observer
void Game_AddObserver(GameSubject *subject, GameObserver *observer) {
    observer->next = subject->observers;
    subject->observers = observer;
}

// Notify all observers
void Game_NotifyObservers(GameSubject *subject, GameEvent event, const ChessBoard *board) {
    GameObserver *observer = subject->observers;
    while (observer != NULL) {
        observer->onGameEvent(event, board);
        observer = observer->next;
    }
}

// Example observers
void uiUpdateObserver(GameEvent event, const ChessBoard *board) {
    if (event == EVENT_MOVE_MADE) {
        UI_Refresh(board);
    }
}

void soundObserver(GameEvent event, const ChessBoard *board) {
    if (event == EVENT_MOVE_MADE) {
        Sound_PlayMoveSound();
    } else if (event == EVENT_CHECK) {
        Sound_PlayCheckSound();
    }
}
```

### 4. Command Pattern - Undo/Redo System

```c
/**
 * Encapsulate moves as command objects for undo/redo
 */

typedef struct Command {
    void (*execute)(ChessBoard *board, struct Command *cmd);
    void (*undo)(ChessBoard *board, struct Command *cmd);
    Move move;
    Piece capturedPiece;
    int previousCastleRights;
    SquareIndex previousEnPassant;
} Command;

typedef struct {
    Command *history[CHESS_MAX_GAME_MOVES];
    int historyCount;
    int currentPosition;
} CommandHistory;

// Execute move command
void Command_Execute(Command *cmd, ChessBoard *board) {
    // Store state for undo
    cmd->capturedPiece = board->pieces[cmd->move.toSquare];
    cmd->previousCastleRights = board->castleRights;
    cmd->previousEnPassant = board->enPassantSquare;
    
    // Execute move
    Move_Execute(board, cmd->move);
}

// Undo move command
void Command_Undo(Command *cmd, ChessBoard *board) {
    Move_TakeBack(board, cmd->move);
    board->pieces[cmd->move.toSquare] = cmd->capturedPiece;
    board->castleRights = cmd->previousCastleRights;
    board->enPassantSquare = cmd->previousEnPassant;
}

// Command history management
void CommandHistory_AddAndExecute(CommandHistory *history, Command *cmd, ChessBoard *board) {
    // Remove any commands after current position (redo history)
    history->historyCount = history->currentPosition;
    
    // Execute and add to history
    Command_Execute(cmd, board);
    history->history[history->historyCount++] = cmd;
    history->currentPosition = history->historyCount;
}

bool CommandHistory_Undo(CommandHistory *history, ChessBoard *board) {
    if (history->currentPosition == 0) {
        return false;
    }
    
    Command *cmd = history->history[--history->currentPosition];
    Command_Undo(cmd, board);
    return true;
}

bool CommandHistory_Redo(CommandHistory *history, ChessBoard *board) {
    if (history->currentPosition >= history->historyCount) {
        return false;
    }
    
    Command *cmd = history->history[history->currentPosition++];
    Command_Execute(cmd, board);
    return true;
}
```

### 5. Singleton Pattern - Engine Options

```c
/**
 * Global engine configuration accessible throughout the application
 */

typedef struct {
    bool useOpeningBook;
    int hashTableSizeMB;
    int maxSearchDepth;
    int searchTimeLimit;
    bool showThinkingOutput;
} EngineOptions;

// Singleton instance
static EngineOptions *g_engineOptions = NULL;

// Get singleton instance
EngineOptions* EngineOptions_GetInstance(void) {
    if (g_engineOptions == NULL) {
        g_engineOptions = (EngineOptions*)malloc(sizeof(EngineOptions));
        
        // Default values
        g_engineOptions->useOpeningBook = true;
        g_engineOptions->hashTableSizeMB = 64;
        g_engineOptions->maxSearchDepth = 20;
        g_engineOptions->searchTimeLimit = 5000;  // 5 seconds
        g_engineOptions->showThinkingOutput = false;
    }
    return g_engineOptions;
}

// Cleanup
void EngineOptions_Destroy(void) {
    if (g_engineOptions != NULL) {
        free(g_engineOptions);
        g_engineOptions = NULL;
    }
}

// Usage
EngineOptions *options = EngineOptions_GetInstance();
if (options->useOpeningBook) {
    // Use opening book
}
```

---

## Migration Roadmap

### Phase 1: Preparation and Analysis (Week 1)

**Goals:**
- Understand current architecture
- Create comprehensive test suite for existing functionality
- Document all current features and edge cases

**Tasks:**
1. Create integration tests that verify complete game functionality
2. Document current API surface
3. Set up version control branching strategy
4. Create `REFACTORING_LOG.md` to track changes

### Phase 2: Type System Refactoring (Week 2)

**Goals:**
- Establish new type definitions
- Create transitional layer for gradual migration

**Tasks:**
1. Create `chess_types.h` with new type definitions
2. Create wrapper functions that accept both old and new types
3. Update documentation with new types
4. Begin converting internal implementations

**Example Transitional Code:**
```c
// chess_types.h - New types
typedef struct { /* ... */ } ChessBoard;
typedef struct { /* ... */ } Move;

// legacy_adapter.h - Transitional layer
#define S_BOARD ChessBoard
#define S_MOVE Move

// Gradually replace in implementation files
void Board_Initialize(ChessBoard *board);  // New
void ResetBoard(S_BOARD *pos);            // Old - wraps new function
```

### Phase 3: File Structure Reorganization (Week 3)

**Goals:**
- Move files to new directory structure
- Update include paths
- Maintain buildability at each step

**Tasks:**
1. Create new directory structure
2. Move files one module at a time
3. Update `#include` statements
4. Update build system (Makefile)
5. Test after each module migration

**Migration Order:**
```
1. core/types/       (foundation)
2. core/board/       (minimal dependencies)
3. core/bitboards/   (depends on types)
4. core/attack/      (depends on board)
5. core/moves/       (depends on attack)
6. engine/           (depends on core)
7. ui/               (depends on everything)
```

### Phase 4: Function and Variable Renaming (Week 4)

**Goals:**
- Apply new naming conventions
- Maintain backwards compatibility where needed

**Tasks:**
1. Create find-replace script for systematic renaming
2. Rename one module at a time
3. Run tests after each module
4. Update documentation

**Systematic Approach:**
```bash
# Example script: rename_functions.sh
#!/bin/bash

# Rename with sed (use with caution, test first!)
find src/core/board -type f -name "*.c" -o -name "*.h" | \
xargs sed -i 's/ResetBoard/Board_Initialize/g'

# After each rename, rebuild and test
make clean && make && ./run_tests.sh
```

### Phase 5: SOLID Principles Implementation (Week 5-6)

**Goals:**
- Refactor large functions into smaller, focused units
- Introduce interfaces for key abstractions
- Decouple tightly coupled modules

**Tasks:**
1. Identify god functions and split them
2. Extract interfaces for polymorphism
3. Introduce dependency injection
4. Create facade patterns for complex subsystems

### Phase 6: Design Patterns Integration (Week 7)

**Goals:**
- Apply identified design patterns
- Improve extensibility and maintainability

**Tasks:**
1. Implement Factory for board creation
2. Implement Strategy for search algorithms
3. Implement Observer for game events
4. Implement Command for move history
5. Document pattern usage

### Phase 7: Testing and Documentation (Week 8)

**Goals:**
- Comprehensive test coverage
- Complete documentation
- Performance benchmarking

**Tasks:**
1. Write unit tests for all modules
2. Write integration tests for key workflows
3. Update API documentation
4. Create architecture diagrams
5. Performance comparison (before/after)
6. Create migration guide for future students

---

## Before & After Examples

### Example 1: Board Initialization

#### Before
```c
// defs.h
typedef struct {
    int pieces[BRD_SQ_NUM];
    U64 pawns[3];
    int KingSq[2];
    int side;
    int enPas;
    int fiftyMove;
    int ply;
    int hisPly;
    int castlePerm;
    U64 posKey;
    int pceNum[13];
    int bigPce[2];
    int majPce[2];
    int minPce[2];
    int material[2];
    S_UNDO history[MAXGAMEMOVES];
    int pList[13][10];
    S_HASHTABLE HashTable[1];
    int PvArray[MAXDEPTH];
    int searchHistory[13][BRD_SQ_NUM];
    int searchKillers[2][MAXDEPTH];
    int capturedWhite[16];
    int capturedBlack[16];
    int capturedWhiteCount;
    int capturedBlackCount;
} S_BOARD;

// board.c
void ResetBoard(S_BOARD *pos) {
    int index = 0;
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        pos->pieces[index] = OFFBOARD;
    }
    for(index = 0; index < 64; ++index) {
        pos->pieces[SQ120(index)] = EMPTY;
    }
    for(index = 0; index < 2; ++index) {
        pos->bigPce[index] = 0;
        pos->majPce[index] = 0;
        pos->minPce[index] = 0;
        pos->material[index] = 0;
    }
    for(index = 0; index < 3; ++index) {
        pos->pawns[index] = 0ULL;
    }
    for(index = 0; index < 13; ++index) {
        pos->pceNum[index] = 0;
    }
    pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;
    pos->side = BOTH;
    pos->enPas = NO_SQ;
    pos->fiftyMove = 0;
    pos->ply = 0;
    pos->hisPly = 0;
    pos->castlePerm = 0;
    pos->posKey = 0ULL;
}
```

#### After
```c
// chess_types.h
typedef enum {
    PIECE_TYPE_NONE = 0,
    PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING
} PieceType;

typedef enum {
    PIECE_COLOR_WHITE = 0,
    PIECE_COLOR_BLACK,
    PIECE_COLOR_NONE
} PieceColor;

typedef struct {
    PieceType type;
    PieceColor color;
} Piece;

typedef struct {
    Piece pieces[BOARD_SQUARE_COUNT];          // Simplified: just 64 squares
    uint64_t pawnBitboards[3];                 // [white, black, both]
    SquareIndex kingSquares[2];                // [white, black]
    PieceColor sideToMove;
    SquareIndex enPassantSquare;
    int halfMoveClock;                         // For fifty-move rule
    int fullMoveNumber;
    CastleRights castleRights;
    uint64_t zobristHash;
} BoardState;

typedef struct {
    BoardState currentState;
    BoardState history[CHESS_MAX_GAME_MOVES];
    int historyCount;
    
    // Derived data (for efficiency, maintained incrementally)
    int pieceCount[2];                         // Total pieces per color
    int material[2];                           // Material score per color
    PieceList pieceLists[PIECE_TYPE_KING + 1][2];  // Organized by type and color
} ChessBoard;

// board_initialization.c
/**
 * @brief Initializes a chess board to empty state
 * @param board Board to initialize
 */
void Board_InitializeEmpty(ChessBoard *board) {
    if (board == NULL) {
        return;
    }
    
    // Zero-initialize board state
    memset(&board->currentState, 0, sizeof(BoardState));
    
    // Set all squares to empty
    for (int square = 0; square < BOARD_SQUARE_COUNT; square++) {
        board->currentState.pieces[square].type = PIECE_TYPE_NONE;
        board->currentState.pieces[square].color = PIECE_COLOR_NONE;
    }
    
    // Initialize derived data
    board->currentState.sideToMove = PIECE_COLOR_WHITE;
    board->currentState.enPassantSquare = SQUARE_NONE;
    board->currentState.halfMoveClock = 0;
    board->currentState.fullMoveNumber = 1;
    board->currentState.castleRights = CASTLE_NONE;
    
    board->historyCount = 0;
    memset(board->pieceCount, 0, sizeof(board->pieceCount));
    memset(board->material, 0, sizeof(board->material));
}

/**
 * @brief Initializes a chess board to starting position
 * @param board Board to initialize
 */
void Board_InitializeStartPosition(ChessBoard *board) {
    Board_InitializeEmpty(board);
    
    // Set up starting position using FEN
    const char *startFEN = CHESS_STARTING_POSITION_FEN;
    StatusCode status = Board_ParseFromFEN(startFEN, board);
    
    if (status != STATUS_SUCCESS) {
        // This should never happen with valid starting FEN
        assert(false && "Failed to parse starting position FEN");
    }
}
```

### Example 2: Move Generation

#### Before
```c
// movegen.c
void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {
    ASSERT(CheckBoard(pos));
    list->count = 0;
    int pce;
    int side = pos->side;
    int sq = 0;
    int t_sq = 0;
    int pceIndex;
    
    if(side == WHITE) {
        for(pceIndex = 0; pceIndex < pos->pceNum[wP]; ++pceIndex) {
            sq = pos->pList[wP][pceIndex];
            ASSERT(SqOnBoard(sq));
            
            if(pos->pieces[sq + 10] == EMPTY) {
                AddWhitePawnMove(pos, sq, sq+10, list);
                if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
                    AddQuietMove(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,MFLAGPS), list);
                }
            }
            
            if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
            }
            if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
            }
            // ... (100+ more lines for all pieces)
        }
    }
    // ... (100+ more lines for black pieces)
}
```

#### After
```c
// move_generation.h
/**
 * @brief Generates all legal moves for the current position
 * @param board Current board state
 * @param moveList Output list of legal moves
 * @return Number of legal moves generated
 */
int MoveGen_GenerateAllLegalMoves(const ChessBoard *board, MoveList *moveList);

// move_generation_pawn.c
/**
 * @brief Generates all legal pawn moves for a given color
 * @param board Current board state
 * @param color Color of pawns to generate moves for
 * @param moveList Output list to append moves to
 * @return Number of pawn moves generated
 */
static int generatePawnMoves(
    const ChessBoard *board,
    PieceColor color,
    MoveList *moveList
) {
    int movesGenerated = 0;
    const int direction = (color == PIECE_COLOR_WHITE) ? 8 : -8;
    const int startRank = (color == PIECE_COLOR_WHITE) ? 1 : 6;
    const int promotionRank = (color == PIECE_COLOR_WHITE) ? 7 : 0;
    
    // Iterate through all pawns of this color
    const PieceList *pawnList = &board->pieceLists[PIECE_TYPE_PAWN][color];
    for (int i = 0; i < pawnList->count; i++) {
        SquareIndex fromSquare = pawnList->squares[i];
        SquareIndex toSquare;
        
        // Forward move
        toSquare = fromSquare + direction;
        if (Board_IsSquareEmpty(board, toSquare)) {
            if (Square_GetRank(toSquare) == promotionRank) {
                movesGenerated += addPromotionMoves(fromSquare, toSquare, moveList);
            } else {
                movesGenerated += addQuietMove(fromSquare, toSquare, moveList);
            }
            
            // Double push from starting rank
            if (Square_GetRank(fromSquare) == startRank) {
                toSquare = fromSquare + (direction * 2);
                if (Board_IsSquareEmpty(board, toSquare)) {
                    Move move = Move_Create(fromSquare, toSquare, MOVE_FLAG_PAWN_DOUBLE);
                    movesGenerated += MoveList_Add(moveList, move);
                }
            }
        }
        
        // Captures (left and right)
        movesGenerated += generatePawnCaptures(board, color, fromSquare, moveList);
        
        // En passant
        if (board->currentState.enPassantSquare != SQUARE_NONE) {
            movesGenerated += generateEnPassantCapture(
                board, color, fromSquare, moveList
            );
        }
    }
    
    return movesGenerated;
}

// move_generation.c
int MoveGen_GenerateAllLegalMoves(const ChessBoard *board, MoveList *moveList) {
    assert(board != NULL);
    assert(moveList != NULL);
    assert(Board_Validate(board));
    
    MoveList_Clear(moveList);
    PieceColor sideToMove = board->currentState.sideToMove;
    
    int movesGenerated = 0;
    movesGenerated += generatePawnMoves(board, sideToMove, moveList);
    movesGenerated += generateKnightMoves(board, sideToMove, moveList);
    movesGenerated += generateBishopMoves(board, sideToMove, moveList);
    movesGenerated += generateRookMoves(board, sideToMove, moveList);
    movesGenerated += generateQueenMoves(board, sideToMove, moveList);
    movesGenerated += generateKingMoves(board, sideToMove, moveList);
    movesGenerated += generateCastlingMoves(board, sideToMove, moveList);
    
    return movesGenerated;
}
```

### Example 3: Search Function

#### Before
```c
// search.c
int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {
    ASSERT(CheckBoard(pos));
    
    if(depth <= 0) {
        return Quiescence(alpha, beta, pos, info);
    }
    
    if(( info->nodes & 2047 ) == 0) {
        ReadInput(info);
    }
    
    info->nodes++;
    
    if((IsRep(pos) || pos->fiftyMove >= 100) && pos->ply) {
        return 0;
    }
    
    if(pos->ply > MAXDEPTH - 1) {
        return EvalPosition(pos);
    }
    
    int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);
    
    if(InCheck == TRUE) {
        depth++;
    }
    
    int Score = -INFINITE;
    int PvMove = ProbePvMove(pos);
    
    // ... (100+ more lines)
}
```

#### After
```c
// search_algorithm.h
/**
 * @brief Search configuration and context
 */
typedef struct {
    int maxDepth;
    int timeLimit;
    int64_t maxNodes;
    bool useNullMove;
    bool useTranspositionTable;
} SearchConfig;

typedef struct {
    int64_t nodesSearched;
    int64_t startTime;
    bool shouldStop;
    float branchingFactor;
    int nullMoveCutoffs;
} SearchStatistics;

// search_alphabeta.c
/**
 * @brief Alpha-beta search with null move pruning
 * 
 * Performs a principal variation search using alpha-beta pruning
 * with various enhancements including null move, transposition table,
 * and iterative deepening.
 * 
 * @param board Current position to search
 * @param depth Remaining depth to search
 * @param alpha Lower bound of search window
 * @param beta Upper bound of search window
 * @param config Search configuration options
 * @param stats Statistics collector
 * @return Evaluation score from current position
 */
static int search_AlphaBeta(
    const ChessBoard *board,
    int depth,
    int alpha,
    int beta,
    const SearchConfig *config,
    SearchStatistics *stats
) {
    assert(Board_Validate(board));
    
    // Check if we should stop searching
    if (Search_ShouldStop(stats, config)) {
        stats->shouldStop = true;
        return 0;
    }
    
    // Leaf node - evaluate position
    if (depth <= 0) {
        return Search_Quiescence(board, alpha, beta, config, stats);
    }
    
    // Update statistics
    stats->nodesSearched++;
    if ((stats->nodesSearched & 2047) == 0) {
        Search_CheckForInput(stats);
    }
    
    // Check for draws
    if (Board_IsDraw(board)) {
        return EVALUATION_DRAW;
    }
    
    // Check for maximum ply
    if (board->currentState.ply >= CHESS_MAX_SEARCH_PLY) {
        return Evaluation_EvaluatePosition(board);
    }
    
    // Extend search if in check
    bool inCheck = Square_IsUnderAttack(
        board->currentState.kingSquares[board->currentState.sideToMove],
        Board_GetOpponentColor(board),
        board
    );
    if (inCheck) {
        depth++;
    }
    
    // Probe transposition table
    Move hashMove = MOVE_NONE;
    int hashScore;
    if (config->useTranspositionTable) {
        TranspositionTableEntry entry;
        if (HashTable_Probe(&entry, board->currentState.zobristHash)) {
            hashMove = entry.bestMove;
            if (entry.depth >= depth) {
                if (TranspositionTable_CanUseCutoff(&entry, alpha, beta)) {
                    return entry.score;
                }
            }
        }
    }
    
    // Null move pruning
    if (config->useNullMove && !inCheck && depth >= NULL_MOVE_DEPTH_THRESHOLD) {
        int nullMoveScore = tryNullMovePruning(
            board, depth, alpha, beta, config, stats
        );
        if (nullMoveScore >= beta) {
            stats->nullMoveCutoffs++;
            return beta;
        }
    }
    
    // Generate and search moves
    MoveList moveList;
    MoveGen_GenerateAllLegalMoves(board, &moveList);
    MoveOrdering_OrderMoves(&moveList, board, hashMove);
    
    int bestScore = -EVALUATION_INFINITE;
    Move bestMove = MOVE_NONE;
    bool foundPV = false;
    
    for (int i = 0; i < moveList.moveCount; i++) {
        Move move = moveList.moves[i].move;
        
        // Make move
        ChessBoard nextBoard;
        Board_Copy(&nextBoard, board);
        if (!Move_MakeMove(&nextBoard, move)) {
            continue;  // Illegal move
        }
        
        int score;
        if (foundPV) {
            // Search with null window
            score = -search_AlphaBeta(&nextBoard, depth - 1, -alpha - 1, -alpha, config, stats);
            
            // Re-search if it failed high
            if (score > alpha && score < beta) {
                score = -search_AlphaBeta(&nextBoard, depth - 1, -beta, -alpha, config, stats);
            }
        } else {
            // Full window search for first move
            score = -search_AlphaBeta(&nextBoard, depth - 1, -beta, -alpha, config, stats);
        }
        
        // Update best score
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            
            if (score > alpha) {
                alpha = score;
                foundPV = true;
                
                // Beta cutoff
                if (score >= beta) {
                    // Update history and killer moves
                    Search_UpdateHistory(move, depth);
                    
                    // Store in transposition table
                    if (config->useTranspositionTable) {
                        HashTable_Store(
                            board->currentState.zobristHash,
                            bestMove,
                            bestScore,
                            depth,
                            HASH_FLAG_BETA
                        );
                    }
                    
                    return beta;
                }
            }
        }
    }
    
    // Store in transposition table
    if (config->useTranspositionTable) {
        HashFlag flag = foundPV ? HASH_FLAG_EXACT : HASH_FLAG_ALPHA;
        HashTable_Store(
            board->currentState.zobristHash,
            bestMove,
            bestScore,
            depth,
            flag
        );
    }
    
    return bestScore;
}
```

---

## Conclusion

### Key Takeaways

1. **Consistency is King**: Apply naming and style conventions uniformly
2. **Gradual Migration**: Refactor incrementally, testing at each step
3. **Documentation Matters**: Well-documented code is maintainable code
4. **SOLID Principles**: Guide architectural decisions for better design
5. **Design Patterns**: Use proven solutions for common problems

### Expected Benefits

**Code Quality:**
- Improved readability and understandability
- Easier debugging and maintenance
- Better testability

**Development Velocity:**
- Faster onboarding for new developers
- Reduced time to implement new features
- Easier code reviews

**Software Engineering Skills:**
- Practical experience with industry standards
- Understanding of architectural patterns
- Appreciation for clean code principles

### Next Steps

1. **Review this document** with your team/instructor
2. **Create a refactoring branch** in version control
3. **Start with Phase 1** (testing and analysis)
4. **Commit frequently** with clear messages
5. **Document lessons learned** as you go

### Resources for Further Learning

- **Books:**
  - "Clean Code" by Robert C. Martin
  - "Design Patterns" by Gang of Four
  - "Code Complete" by Steve McConnell

- **Online:**
  - [NASA C Style Guide](https://ntrs.nasa.gov/citations/19950022400)
  - [Linux Kernel Coding Style](https://www.kernel.org/doc/html/latest/process/coding-style.html)
  - [MISRA C Guidelines](https://www.misra.org.uk/)

---

**Remember:** The goal isn't perfection on the first try. The goal is continuous improvement and learning to write maintainable, professional code. Each refactoring makes the codebase a little better, and each lesson learned makes you a better developer.
