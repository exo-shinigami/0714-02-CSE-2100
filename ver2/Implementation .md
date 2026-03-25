# Gambit Chess Engine: Refactoring Report (ver1 → ver2)

**Course:** Advanced Programming Lab — 2nd Year CSE  
**Project:** Gambit Chess Engine  
**Date:** February 2026

---

## Table of Contents

### Part A — AI Prompts Used
1. [Overview](#overview)
2. [Stage 1: Analysis & Planning Prompts](#stage-1-analysis--planning-prompts)
3. [Stage 2: Coding Convention & Standards Prompts](#stage-2-coding-convention--standards-prompts)
4. [Stage 3: Folder Structure Reorganization Prompts](#stage-3-folder-structure-reorganization-prompts)
5. [Stage 4: Code-Level Improvement Prompts](#stage-4-code-level-improvement-prompts)
6. [Stage 5: Documentation & Guide Prompts](#stage-5-documentation--guide-prompts)
7. [Prompt Design Philosophy](#prompt-design-philosophy)
8. [Summary of Prompt Categories](#summary-of-prompt-categories)

### Part B — Detailed Explanation of Changes
9. [Executive Summary](#9-executive-summary)
10. [Coding & Naming Conventions](#10-coding--naming-conventions)
11. [Design Model](#11-design-model)
12. [SOLID Principles Analysis](#12-solid-principles-analysis)
13. [Design Patterns Analysis](#13-design-patterns-analysis)
14. [Detailed Change Log (ver1 → ver2)](#14-detailed-change-log-ver1--ver2)
15. [What Was Improved & What Remains](#15-what-was-improved--what-remains)
16. [Lessons Learned](#16-lessons-learned)
17. [Appendix A: File Metrics Comparison](#appendix-a-file-metrics-comparison)
18. [Appendix B: Dependency Direction Verification](#appendix-b-dependency-direction-verification)

---

# PART A — AI PROMPTS USED

---

## Overview

This section records the AI prompts we used to transform the Gambit Chess Engine from its original monolithic structure (ver1) to the modular, well-documented version (ver2). The refactoring was done in stages, each guided by specific prompts that targeted coding conventions, SOLID principles, design patterns, and structural reorganization.

---

## Stage 1: Analysis & Planning Prompts

### Prompt 1.1 — Initial Codebase Audit

> "Analyze this C chess engine codebase. Read all source files (defs.h, gambit.c, board.c, movegen.c, makemove.c, search.c, evaluate.c, attack.c, bitboards.c, data.c, hashkeys.c, init.c, io.c, misc.c, pvtable.c, perft.c, polybook.c, uci.c, xboard.c, gui.c, gui.h, validate.c). For each file, list all functions, global variables, data structures, and macros. Identify naming conventions, SOLID principle violations, missing design patterns, and code smells. Provide a comprehensive architecture summary."

**Purpose:** Understanding the existing codebase before making any changes. We needed to know what we were working with — every function signature, every global variable, every struct — so we could plan the refactoring intelligently.

**What we learned:**
- The codebase had ~75 public functions across 20 `.c` files, all in a flat directory
- One monolithic header (`defs.h`) contained ALL type definitions, macros, and extern declarations
- `S_BOARD` was a "god object" with 25+ fields mixing board state, search state, and GUI data
- Naming was inconsistent: PascalCase functions, camelCase locals, heavy abbreviations (`pce`, `sq`, `Prom`)
- No design patterns were formally implemented despite natural opportunities for Strategy, Observer, and Command patterns

### Prompt 1.2 — SOLID Violations Identification

> "Based on the chess engine analysis, identify specific violations of each SOLID principle (Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion). For each violation, explain the problem using the actual code (reference specific files and functions), explain why it's a problem, and propose a concrete solution following C programming conventions."

**Purpose:** To create an actionable list of design problems that could be addressed systematically. This directly fed into the REFACTORING_GUIDE.md document.

**Key violations identified:**
- **SRP:** `S_BOARD` holds board state + search state + GUI state; `gui.c` (1400+ lines) handles rendering, input, game logic, timers, promotion, and AI invocation
- **OCP:** Adding new UI modes requires modifying `gambit.c`'s if/else chain; evaluation has no extension mechanism
- **ISP:** Every module includes the same `defs.h` and gets access to every function declaration
- **DIP:** `gui.c` directly calls `EvalPosition()`, `MakeMove()`, `GenerateAllMoves()` with no abstraction layer

### Prompt 1.3 — Design Pattern Opportunities

> "For this chess engine in C, identify which Gang of Four design patterns could improve the architecture. Show concrete before/after code examples for: (1) Factory pattern for board creation, (2) Strategy pattern for search algorithms and move ordering, (3) Observer pattern for game state notifications, (4) Command pattern for undo/redo, and (5) Singleton pattern for engine options. Use C-style function-pointer structs to simulate interfaces."

**Purpose:** To understand how design patterns apply in a C codebase (not C++/Java where they're typically taught). We needed concrete examples that show function-pointer-based polymorphism in C.

---

## Stage 2: Coding Convention & Standards Prompts

### Prompt 2.1 — Naming Convention Design

> "Create a comprehensive naming convention document for a C chess engine project. Cover: (1) Variable naming (local, global, constants), (2) Function naming with module prefixes, (3) Struct/typedef naming, (4) Enum naming, (5) Macro naming, (6) File naming. Show before/after examples using actual names from this codebase: S_BOARD, ParseFen, SqAttacked, pceNum, FROMSQ, GenerateAllMoves. The convention should be consistent, descriptive, and follow industry C coding standards."

**Purpose:** To establish a project-specific coding convention that addresses the existing inconsistencies. This became Section 2 ("New Naming Conventions") of our REFACTORING_GUIDE.

**Convention decisions made:**
| Element | Convention | Example |
|---------|-----------|---------|
| Types/Structs | PascalCase, no `S_` prefix | `ChessBoard`, `ScoredMove` |
| Public Functions | `Module_VerbNoun` PascalCase | `Board_ParseFromFEN()` |
| Local Variables | camelCase, descriptive | `squareIndex`, `temporaryPieceCount` |
| Global Variables | `g_` prefix + camelCase | `g_square120To64[]` |
| Constants/Macros | `MODULE_UPPER_SNAKE_CASE` | `CHESS_MAX_GAME_MOVES`, `MOVE_GET_FROM_SQUARE()` |
| Enum Values | `TYPE_UPPER_SNAKE_CASE` | `PIECE_TYPE_PAWN`, `PIECE_COLOR_WHITE` |
| File Names | `module_component.c` | `board_representation.c` |

### Prompt 2.2 — Documentation Standard

> "Create a Doxygen-style documentation standard for a C project. Include templates for: (1) File headers with @file, @brief, @author, @date, @version, (2) Function documentation with @param, @return, @note, @complexity, (3) Inline comment guidelines (explain 'why' not 'what'), (4) Block comment format for algorithm explanations. Show examples using chess engine functions like SqAttacked, GenerateAllMoves, AlphaBeta."

**Purpose:** To add professional documentation to every file and function declaration, making the codebase self-documenting.

---

## Stage 3: Folder Structure Reorganization Prompts

### Prompt 3.1 — Module Architecture Design

> "Design a modular folder structure for a C chess engine with these components: board representation, move generation, move execution, attack detection, bitboard operations, search algorithm, position evaluation, transposition table, performance testing, opening book, UCI protocol, XBoard protocol, SDL2 GUI, initialization utilities, and miscellaneous utilities. The structure should enforce separation of concerns with clear dependency directions: UI → Engine → Core. Include a makefile that mirrors the source tree in build output."

**Purpose:** This was the core structural transformation. We needed a folder hierarchy that physically separates concerns and makes the dependency architecture visible.

**Result — the implemented structure:**
```
src/
├── main/           → Entry point (gambit.c)
├── core/           → Pure chess logic (no UI dependencies)
│   ├── types/      → Type definitions (defs.h, data.c)
│   ├── board/      → Board state (board.c, hashkeys.c, validate.c)
│   ├── moves/      → Move operations (movegen.c, makemove.c, io.c)
│   ├── bitboards/  → Bitboard utilities (bitboards.c)
│   └── attack/     → Attack detection (attack.c)
├── engine/         → AI components
│   ├── search/     → Search algorithms (search.c, perft.c)
│   ├── evaluation/ → Position evaluation (evaluate.c)
│   └── hashtable/  → Transposition table (pvtable.c)
├── ui/             → Presentation layer
│   ├── sdl/        → SDL2 GUI (gui.c, gui.h)
│   ├── protocols/  → UCI/XBoard (uci.c, xboard.c)
│   └── console/    → Console mode (placeholder)
├── utils/          → Cross-cutting utilities (init.c, misc.c)
└── openingbook/    → External data (polybook.c, polykeys.c/h)
```

### Prompt 3.2 — File Migration Execution

> "I need to move all source files from a flat directory structure into the modular folder structure we designed. For each source file, tell me: (1) Which folder it moves to, (2) What #include path changes are needed, (3) Any other modifications needed for the file to compile in its new location. Then generate an updated makefile that compiles all source files from their new locations, outputs object files to build/obj/ mirroring the source tree, and links to build/bin/gambit.exe."

**Purpose:** To execute the actual file reorganization without breaking compilation. Every `#include` path had to be updated, and the makefile needed a complete rewrite.

**Key changes applied:**
- All 22 source files moved to their designated subdirectories
- `#include "defs.h"` paths updated to reference `src/core/types/defs.h` via `-I` flags
- `#include "gui.h"` paths updated for files that need the GUI header
- Makefile rewritten with `SOURCES_CORE` and `SOURCES_GUI` separation
- Object files output to `build/obj/src/module/submodule/file.o`
- Conditional GUI compilation with `-DENABLE_GUI` flag

### Prompt 3.3 — Build System Modernization

> "Update the makefile for a chess engine with source files organized under src/ in multiple subdirectories. Requirements: (1) Static linking of SDL2 and SDL2_ttf (eliminate DLL dependencies), (2) Object files in build/obj/ mirroring source tree, (3) Binary in build/bin/ with copy to project root, (4) Targets: all, clean, rebuild, run, directories, help, (5) Conditional GUI compilation via ENABLE_GUI flag, (6) Support for Windows (MinGW) compilation with -mwindows flag."

**Purpose:** To create a professional build system that supports the new structure and eliminates the need to distribute SDL2 DLLs alongside the executable.

---

## Stage 4: Code-Level Improvement Prompts

### Prompt 4.1 — Adding Doxygen Headers to All Files

> "Add professional Doxygen-style file headers to every source file in the chess engine. Each header should include: @file (with relative path from src/), @brief (one-line purpose), a longer description paragraph explaining the module's role, @author, @date, @version. Also add @brief documentation to every function declaration in defs.h. Here are all the files: [list of all 22 files with their purposes]."

**Purpose:** To bring every file up to professional documentation standards. This was applied to both the `.c` files and all ~46 function declarations in `defs.h`.

### Prompt 4.2 — GUI Integration (New Feature)

> "Add the following features to the SDL2 chess GUI: (1) Captured pieces display with black pieces in a left column and white pieces in a right column, (2) Pawn promotion dialog showing all 4 piece choices (Queen, Rook, Bishop, Knight) with visual selection, (3) Legal move highlighting when a piece is selected (green overlay on valid destination squares), (4) King check highlighting (red square when king is in check). Track captured pieces in the S_BOARD struct by adding capturedWhite[16], capturedBlack[16], and counts. Update MakeMove/TakeMove to maintain these arrays."

**Purpose:** To enhance the GUI with features that demonstrate event handling, state tracking, and visual feedback — features that naturally showcase the Observer and State patterns in practice.

### Prompt 4.3 — Search API for GUI

> "Add a new function GetBestMove() to search.c that runs a shallow search (limited to a few seconds) and returns the best move without executing it. This function should be callable from the GUI's HandleMouseClick when it's the engine's turn in PvE mode. It should use the existing SearchPosition infrastructure but be designed as a clean API call that the GUI layer can use without knowing the search internals."

**Purpose:** To create a cleaner interface between the UI and engine layers — a step toward Dependency Inversion by providing a simple API function rather than having the GUI directly manipulate search internals.

---

## Stage 5: Documentation & Guide Prompts

### Prompt 5.1 — Refactoring Guide Creation

> "Create a comprehensive refactoring guide document for a C chess engine project. Include sections on: (1) Executive summary of current state vs. desired state, (2) New naming conventions with before/after examples, (3) New folder structure with rationale, (4) Programming style guidelines (indentation, function length, parameter limits, error handling), (5) SOLID principles with chess-engine-specific C code examples, (6) Design patterns with full C implementation examples using function pointers, (7) An 8-phase migration roadmap with weekly milestones, (8) Before/after code examples for board init, move generation, and search. Mark Phase 3 (file reorganization) as completed."

**Purpose:** To create a living document that serves both as a record of what was done and a roadmap for future improvements. The guide shows understanding of concepts even for parts not yet fully implemented in code.

### Prompt 5.2 — Before/After Code Examples

> "Write detailed before/after code comparison examples for the chess engine refactoring. Show three examples: (1) Board initialization — from the current ResetBoard function to a decomposed Board_InitializeEmpty + Board_InitializeStartPosition with proper types, (2) Move generation — from the monolithic GenerateAllMoves to decomposed per-piece generators with a color-agnostic approach, (3) Alpha-beta search — from the current function to one using SearchConfig and SearchStatistics structs with clear separation of null-move pruning, transposition table probing, and history updates."

**Purpose:** To demonstrate deep understanding of how SOLID principles and design patterns would transform the actual chess engine code, even where the transformation is planned for future phases.

---

## Prompt Design Philosophy

### Why These Prompts Worked

1. **Context-heavy:** Every prompt referenced actual file names, function names, and struct names from our codebase — not generic examples
2. **Incremental:** We went from analysis → planning → conventions → structure → code → documentation, never skipping steps
3. **Specific outputs:** Each prompt asked for concrete deliverables (code, folder trees, makefile rules) rather than abstract advice
4. **Domain-aware:** We used chess engine terminology (bitboards, alpha-beta, Zobrist hashing, MVV-LVA) so the AI understood the domain constraints
5. **Validation-oriented:** After each structural change, we asked the AI to verify compilation and functionality

### What We Would Do Differently

- **Apply naming conventions in code:** We documented the naming convention thoroughly but only applied it to new GUI code, not the existing engine core. Future work should systematically rename using the `Module_VerbNoun` pattern.
- **Split defs.h earlier:** The monolithic header should have been split into per-module headers (`board.h`, `movegen.h`, `search.h`) during the file reorganization phase.
- **Implement at least one design pattern in code:** The Strategy pattern for UI backends was fully designed in the guide but not implemented. Even a minimal function-pointer-based `UIBackend` struct would have demonstrated the concept in working code.
- **Decompose S_BOARD:** The god object grew larger (added captured piece fields) instead of being split. We should have separated `BoardState`, `SearchState`, and `GUIState` into distinct structs.

---

## Summary of Prompt Categories

| Category | # Prompts | Purpose |
|----------|-----------|---------|
| Analysis & Audit | 3 | Understand codebase, find SOLID violations, identify pattern opportunities |
| Conventions & Standards | 2 | Design naming rules, documentation templates |
| Structural Reorganization | 3 | Design folder hierarchy, migrate files, modernize build |
| Code Improvements | 3 | Add documentation, GUI features, clean API |
| Documentation | 2 | Create refactoring guide, write before/after examples |
| **Total** | **13** | Full refactoring pipeline from analysis to documentation |

---

# PART B — DETAILED EXPLANATION OF CHANGES

---

## 9. Executive Summary

The Gambit Chess Engine was transformed from a **flat, monolithic C codebase** (ver1: 22 source files in a single directory) into a **modular, layered architecture** (ver2: same files organized into a 6-layer folder hierarchy under `src/`). The changes fall into four categories:

| Category | What Changed |
|----------|-------------|
| **Physical Structure** | Flat directory → `src/core/`, `src/engine/`, `src/ui/`, `src/utils/`, `src/openingbook/`, `src/main/` |
| **Build System** | Simple single-target makefile → Structured build with `build/obj/`, `build/bin/`, static linking, conditional compilation |
| **Documentation** | Minimal → Comprehensive Doxygen-style file/function documentation + 1900-line REFACTORING_GUIDE |
| **New Features** | Console-only → Full SDL2 GUI with captured pieces, move highlighting, check highlighting, promotion dialog, timers |

**Core engine logic was NOT modified.** The same functions, same algorithms, same data structures, and same naming exist in ver2 — they are simply organized into logical modules and documented.

---

## 10. Coding & Naming Conventions

### 10.1 Convention System Designed for This Project

We created a naming convention tailored to a **C chess engine** that addresses the specific domain vocabulary (pieces, squares, moves, boards) while following industry standards.

#### The Convention Table

| Element | Rule | Example (ver1 → Proposed) |
|---------|------|--------------------------|
| **Struct types** | PascalCase, no prefixes | `S_BOARD` → `ChessBoard` |
| **Public functions** | `Module_VerbNoun` | `ParseFen()` → `Board_ParseFromFEN()` |
| **Static functions** | camelCase, local scope | `AddQuietMove()` stays internal |
| **Local variables** | camelCase, descriptive | `sq` → `squareIndex` |
| **Global variables** | `g_` prefix + camelCase | `SetMask[]` → `g_bitSetMask[]` |
| **Constants** | `MODULE_UPPER_SNAKE` | `MAXDEPTH` → `CHESS_MAX_SEARCH_DEPTH` |
| **Macros** | `MODULE_ACTION_OBJECT` | `FROMSQ(m)` → `MOVE_GET_FROM_SQUARE(m)` |
| **Enum values** | `TYPE_UPPER_SNAKE` | `wP` → `PIECE_TYPE_PAWN` (with color struct) |
| **File names** | `module_component.c` | `board.c` → `board_representation.c` |

#### Why These Conventions?

1. **Module prefixes** (`Board_`, `Move_`, `Search_`) prevent name collisions in C (no namespaces) and make it immediately clear which subsystem a function belongs to.
2. **The `g_` prefix** for globals is critical in a C project with 40+ global variables — it makes every global reference visually distinct from locals.
3. **Descriptive enum values** (`PIECE_TYPE_PAWN` vs `wP`) are self-documenting. The old `wP` requires knowing the codebase; `PIECE_TYPE_PAWN` is understood by anyone.
4. **Verb-Noun function names** (`Board_ParseFromFEN`) clearly communicate what a function does and what it acts on.

#### Convention Application Status

| Where | Applied? | Details |
|-------|----------|---------|
| New GUI code (`gui.c`, `gui.h`) | **Partially** | PascalCase functions (`InitGUI`, `RenderBoard`, `HandleMouseClick`), camelCase struct fields |
| Core engine files | **Not yet** | All original names preserved (planned for Phase 4) |
| Documentation/Guide | **Fully** | Complete convention system documented with before/after examples |
| File headers (Doxygen) | **Fully** | Applied to all 22 source files and all 46 function declarations |

### 10.2 Documentation Convention

Every source file now begins with a standardized Doxygen header:

```c
/**
 * @file core/board/board.c
 * @brief Core board representation and state management
 *
 * Handles the 10×12 mailbox board representation including piece
 * placement, FEN parsing, board validation, and state reset.
 *
 * @author Team Name
 * @date February 2026
 * @version 2.0
 */
```

Every function declaration in `defs.h` now has documentation:

```c
/**
 * @brief Checks if a square is attacked by a given side
 * @param sq The square to check (120-based index)
 * @param side The attacking side (WHITE or BLACK)
 * @param pos Pointer to the board state
 * @return TRUE if the square is attacked, FALSE otherwise
 */
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);
```

---

## 11. Design Model

### 11.1 Architectural Model: Layered Architecture

The ver2 design follows a **Layered Architecture** pattern, where each layer has a clear responsibility and dependencies flow in one direction:

```
┌─────────────────────────────────────────────────┐
│                  UI Layer (src/ui/)              │
│  ┌──────────┐  ┌──────────┐  ┌───────────────┐  │
│  │ SDL GUI  │  │   UCI    │  │    XBoard     │  │
│  │ (gui.c)  │  │ (uci.c) │  │  (xboard.c)  │  │
│  └────┬─────┘  └────┬─────┘  └──────┬────────┘  │
│       │              │               │           │
├───────┼──────────────┼───────────────┼───────────┤
│       ▼              ▼               ▼           │
│              Engine Layer (src/engine/)           │
│  ┌──────────┐  ┌────────────┐  ┌──────────────┐  │
│  │  Search  │  │ Evaluation │  │  Hash Table  │  │
│  │(search.c)│  │(evaluate.c)│  │ (pvtable.c)  │  │
│  └────┬─────┘  └─────┬──────┘  └──────┬───────┘  │
│       │               │               │          │
├───────┼───────────────┼───────────────┼──────────┤
│       ▼               ▼               ▼          │
│               Core Layer (src/core/)             │
│  ┌────────┐  ┌─────────┐  ┌────────┐  ┌───────┐  │
│  │ Board  │  │  Moves  │  │ Attack │  │Bitbrd │  │
│  │board.c │  │movegen.c│  │attack.c│  │bitbrd │  │
│  │hashk.c │  │makemv.c │  │        │  │  .c   │  │
│  │valid.c │  │  io.c   │  │        │  │       │  │
│  └────┬───┘  └────┬────┘  └───┬────┘  └───┬───┘  │
│       └──────┬─────┘───────────┘───────────┘     │
│              ▼                                    │
│       Types Foundation (src/core/types/)          │
│       ┌────────────────────────────────┐          │
│       │  defs.h (types, macros, decls) │          │
│       │  data.c (lookup tables)        │          │
│       └────────────────────────────────┘          │
│                                                   │
├───────────────────────────────────────────────────┤
│  Cross-cutting: utils/ (init.c, misc.c)          │
│  External data: openingbook/ (polybook.c)        │
│  Entry point: main/ (gambit.c)                    │
└───────────────────────────────────────────────────┘
```

### 11.2 Dependency Rules

| Rule | Description | Example |
|------|------------|---------|
| **Downward only** | Upper layers can call lower layers, never the reverse | `gui.c` calls `MakeMove()` (core), but `makemove.c` never calls GUI functions |
| **Same-layer OK** | Modules in the same layer can call each other | `search.c` calls `EvalPosition()` (both in engine layer) |
| **No skip (mostly)** | UI should go through Engine, not call Core directly | *Partially violated:* GUI calls `MakeMove()` directly for human moves |
| **Foundation is universal** | `defs.h` is included everywhere | All files depend on `core/types/` |

### 11.3 Module Responsibilities

| Module | Folder | Single Responsibility |
|--------|--------|----------------------|
| **Types** | `core/types/` | Define all data types, constants, macros, and global lookup tables |
| **Board** | `core/board/` | Maintain board state: reset, parse FEN, validate, generate hash keys |
| **Moves** | `core/moves/` | Generate legal moves, execute/undo moves, format move I/O |
| **Attack** | `core/attack/` | Determine if a square is attacked by a specific side |
| **Bitboards** | `core/bitboards/` | Low-level bitboard operations (pop bit, count bits) |
| **Search** | `engine/search/` | Alpha-beta search, iterative deepening, quiescence search, perft testing |
| **Evaluation** | `engine/evaluation/` | Static position evaluation with piece-square tables |
| **Hash Table** | `engine/hashtable/` | Transposition table (store, probe, PV extraction) |
| **SDL GUI** | `ui/sdl/` | Graphical chess interface using SDL2 |
| **Protocols** | `ui/protocols/` | UCI and XBoard protocol handling |
| **Utils** | `utils/` | Engine initialization, time measurement, input handling |
| **Opening Book** | `openingbook/` | Polyglot opening book loading and move lookup |
| **Main** | `main/` | Entry point, CLI argument parsing, mode selection |

### 11.4 Data Flow Model

**Human Move in GUI (PvE mode):**
```
User clicks square → gui.c:HandleMouseClick()
    → core/moves/movegen.c:GenerateAllMoves()     [Generate legal moves]
    → core/moves/makemove.c:MakeMove()             [Execute move]
    → gui.c:AddMoveToHistory()                     [Update GUI state]
    → gui.c:SetGameOver() check                    [Check if game ended]
    → engine/search/search.c:GetBestMove()         [Engine responds]
    → core/moves/makemove.c:MakeMove()             [Execute engine move]
    → gui.c:RenderBoard()                          [Redraw]
```

**UCI Protocol Mode:**
```
UCI client sends "position" → ui/protocols/uci.c:ParsePosition()
    → core/board/board.c:ParseFen()
    → core/moves/makemove.c:MakeMove() (for each move)

UCI client sends "go" → ui/protocols/uci.c:ParseGo()
    → engine/search/search.c:SearchPosition()
        → engine/evaluation/evaluate.c:EvalPosition()
        → engine/hashtable/pvtable.c:ProbeHashEntry() / StoreHashEntry()
        → core/moves/movegen.c:GenerateAllMoves()
    → printf("bestmove ...")
```

---

## 12. SOLID Principles Analysis

### 12.1 Single Responsibility Principle (SRP)

**Definition:** A module should have one, and only one, reason to change.

#### How We Applied It — Physical Module Separation

In ver1, all files sat in one directory. There was no structural indication of which files belonged to which subsystem. A developer looking at the file list sees `board.c`, `attack.c`, `uci.c`, `gui.c` — but nothing tells them that `board.c` and `attack.c` are core chess logic while `uci.c` and `gui.c` are presentation.

In ver2, the folder structure itself enforces SRP:

```
ver1 (flat):                    ver2 (modular):
  attack.c                       src/core/attack/attack.c
  board.c                        src/core/board/board.c
  evaluate.c                     src/engine/evaluation/evaluate.c
  gui.c                          src/ui/sdl/gui.c
  uci.c                          src/ui/protocols/uci.c
```

Now each folder has one reason to change:
- `core/board/` changes only when board representation changes
- `engine/search/` changes only when search algorithm changes
- `ui/sdl/` changes only when GUI features change

#### Where SRP Is Still Violated

1. **`S_BOARD` struct** — Still contains board state (pieces, pawns, castlePerm) + search state (searchHistory, searchKillers, PvArray) + GUI state (capturedWhite, capturedBlack). This should be three separate structs.

2. **`defs.h`** — Still contains ALL type definitions for ALL modules. A change to the `GUI` struct's fields forces recompilation of every file in the project because every file includes `defs.h`.

3. **`gui.c`** (1450 lines) — Handles rendering, input processing, game logic, timer management, promotion dialogs, and AI invocation. Should be split into `gui_renderer.c`, `gui_input.c`, `gui_game_logic.c`.

#### Proposed Fix (from REFACTORING_GUIDE)

```c
// Split S_BOARD into focused structs:
typedef struct {
    int pieces[120];
    U64 pawns[3];
    int KingSq[2];
    int side, enPas, fiftyMove, castlePerm;
    U64 posKey;
} BoardState;               // Pure board representation

typedef struct {
    int searchHistory[13][120];
    int searchKillers[2][64];
    int PvArray[64];
    S_HASHTABLE HashTable[1];
} SearchState;               // Search-specific data

typedef struct {
    int capturedWhite[16], capturedBlack[16];
    int capturedWhiteCount, capturedBlackCount;
} CapturedPieces;            // GUI-specific data
```

### 12.2 Open/Closed Principle (OCP)

**Definition:** Software entities should be open for extension but closed for modification.

#### How We Applied It — Folder Structure Enables Extension

The `ui/` folder structure is designed so new UI backends can be added without modifying existing code:

```
src/ui/
├── sdl/          ← SDL2 GUI implementation
├── protocols/    ← UCI + XBoard implementations
└── console/      ← Placeholder for future console UI
```

Adding a new UI (e.g., a web interface) means adding `src/ui/web/` — not modifying `gui.c` or `uci.c`.

#### Where OCP Is Still Violated

The main entry point (`gambit.c`) uses an if/else chain to select UI mode:

```c
// ver2/src/main/gambit.c — closed for extension
if (strcmp(argv[1], "uci") == 0) {
    Uci_Loop(pos, &info);
} else if (strcmp(argv[1], "xboard") == 0) {
    XBoard_Loop(pos, &info);
}
```

#### Proposed Fix — Strategy Pattern via Function Pointers

```c
typedef struct {
    void (*initialize)(void);
    void (*runLoop)(S_BOARD *pos, S_SEARCHINFO *info);
    void (*cleanup)(void);
} UIBackend;

// Register backends — adding a new one doesn't modify this file
UIBackend backends[] = {
    { NULL, Uci_Loop, NULL },
    { SDL_Init, RunGUI, SDL_Cleanup },
    { NULL, Console_Loop, NULL }
};
```

### 12.3 Liskov Substitution Principle (LSP)

**Definition:** Subtypes must be behaviorally substitutable for their base types.

In C, we don't have classes/inheritance, but LSP applies to **function pointer interfaces**. If we define a `UIBackend` interface with `runLoop(pos, info)`, any implementation (UCI, XBoard, SDL, Console) must accept the same inputs and produce correct behavior — no implementation should crash or ignore the `pos` parameter.

The REFACTORING_GUIDE proposes **consistent evaluation interfaces**:

```c
typedef int (*EvaluationFunction)(const S_BOARD *board);

// All follow the same contract: take a board, return centipawn score
int Evaluation_MaterialOnly(const S_BOARD *board);     // Simple
int Evaluation_FullEvaluation(const S_BOARD *board);    // Complex
// Both are interchangeable — LSP satisfied
```

### 12.4 Interface Segregation Principle (ISP)

**Definition:** Clients should not depend on interfaces they don't use.

#### How We Applied It — Layered Structure Reduces Exposure

Before (ver1): `gui.c` includes `defs.h` which exposes ALL 46 function declarations — including `AlphaBeta()`, `ClearHashTable()`, `PerftTest()` — none of which the GUI needs directly.

The folder structure in ver2 establishes logical boundaries. Even though `defs.h` is still monolithic, the folders document which functions each module SHOULD use:

- `ui/sdl/` should only call: `MakeMove`, `TakeMove`, `GenerateAllMoves`, `ParseFen`, `ResetBoard`, `SqAttacked`, `GetBestMove`, `EvalPosition`
- `ui/sdl/` should NOT call: `AlphaBeta`, `Quiescence`, `ClearPiece`, `AddPiece`, `MovePiece`, `ProbeHashEntry`

#### Where ISP Is Still Violated

Every file includes `defs.h` and gets access to everything. The fix is to split `defs.h` into:

```
core/types/chess_types.h      → Piece, Square, Board types
core/board/board.h            → Board management API
core/moves/movegen.h          → Move generation API
engine/search/search.h        → Search API
ui/sdl/gui.h                  → GUI types (already separate!)
```

Note that `gui.h` is already separated — it's the one header that follows ISP.

### 12.5 Dependency Inversion Principle (DIP)

**Definition:** High-level modules should depend on abstractions, not low-level modules.

#### How We Applied It — `GetBestMove()` API

In ver1, when the GUI needed the engine to play a move, it directly called `-EvalPosition(pos)` — a 1-ply evaluation that was extremely weak. This was a tight coupling to a specific low-level function.

In ver2, the new `GetBestMove()` function in `search.c` provides an abstraction:

```c
// ver2/src/engine/search/search.c
int GetBestMove(S_BOARD *pos, S_SEARCHINFO *info) {
    SearchPosition(pos, info);  // Full alpha-beta search
    return pos->PvArray[0];     // Return best move
}
```

The GUI calls `GetBestMove()` without knowing whether it uses alpha-beta, minimax, or Monte Carlo Tree Search internally. This is a step toward DIP — the UI depends on an abstraction (`GetBestMove`) rather than a concrete implementation (`AlphaBeta`).

#### Where DIP Is Still Violated

```c
// gui.c still directly calls concrete functions:
MakeMove(pos, move);           // Direct dependency on core
GenerateAllMoves(pos, &list);  // Direct dependency on core
SqAttacked(sq, side, pos);     // Direct dependency on core
```

The fix would be to inject these as function pointers through the `GUI` struct or a `GameController` interface.

---

## 13. Design Patterns Analysis

### 13.1 Patterns Already Present (ver1 and ver2)

#### Memento Pattern — Move Undo System

The `S_UNDO` struct + `history[]` array implements the Memento pattern. Before each move, the engine saves the board state (castle permissions, en passant, fifty-move counter, position key) into an undo record. `TakeMove()` restores the state from this record.

```c
// makemove.c — Saving memento before move
pos->history[pos->hisPly].posKey = pos->posKey;
pos->history[pos->hisPly].move = move;
pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
pos->history[pos->hisPly].enPas = pos->enPas;
pos->history[pos->hisPly].castlePerm = pos->castlePerm;

// TakeMove() — Restoring from memento
pos->castlePerm = pos->history[pos->hisPly].castlePerm;
pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
pos->enPas = pos->history[pos->hisPly].enPas;
pos->posKey = pos->history[pos->hisPly].posKey;
```

#### Table-Driven Design Pattern

The entire engine is built on lookup tables rather than runtime computation:

| Table | Purpose |
|-------|---------|
| `Sq120ToSq64[120]` / `Sq64ToSq120[64]` | Convert between 120-square and 64-square representations |
| `PieceVal[13]` | Material value of each piece type |
| `PceDir[13][8]` | Movement directions per piece |
| `PawnTable[64]`, `KnightTable[64]`, etc. | Piece-square evaluation tables |
| `CastlePerm[120]` | Bitmask update for castle rights when a square is involved in a move |
| `MvvLvaScores[13][13]` | Most Valuable Victim / Least Valuable Attacker scores for move ordering |

This is a deliberate design choice: **pre-compute everything, look up at runtime**. It's O(1) for type checks and direction lookups, which is critical in a chess engine that evaluates millions of positions per second.

#### Strategy Pattern (Informal) — UI Mode Selection

The engine supports three UI modes (UCI, XBoard, SDL GUI) via separate loop functions:

```c
// gambit.c — Strategy selection
if (strcmp(argv[1], "uci") == 0)
    Uci_Loop(pos, &info);           // Strategy A
else if (strcmp(argv[1], "xboard") == 0)
    XBoard_Loop(pos, &info);        // Strategy B
else
    RunGUI(pos, &info);             // Strategy C (default)
```

This is an *informal* Strategy pattern — the strategies exist but aren't encapsulated behind a common interface.

### 13.2 Patterns Proposed (in REFACTORING_GUIDE)

#### Factory Pattern — Board Creation

Instead of exposing `ResetBoard()` + `ParseFen()` as separate calls, wrap them in a factory:

```c
ChessBoard* BoardFactory_Create(BoardCreationType type, void *params) {
    switch (type) {
        case BOARD_START_POSITION: Board_SetupStartPosition(board); break;
        case BOARD_FROM_FEN: Board_ParseFromFEN(params, board); break;
        case BOARD_EMPTY: Board_SetupEmpty(board); break;
    }
    return board;
}
```

**Why it matters:** Centralizes board construction, ensures derived data (material counts, piece lists, hash key) is always updated after setup.

#### Observer Pattern — Game Events

Instead of the GUI polling for check/checkmate/game over conditions every frame:

```c
// Current: GUI checks every frame
int kingInCheck = SqAttacked(pos->KingSq[pos->side], pos->side^1, pos);

// Proposed: Engine notifies observers
Game_NotifyObservers(subject, EVENT_CHECK, board);    // Triggers UI update
Game_NotifyObservers(subject, EVENT_GAME_OVER, board); // Triggers game over screen
```

**Why it matters:** Decouples the engine from the UI. The engine fires events; the UI subscribes to the ones it cares about.

#### Command Pattern — Move History

Instead of raw integer moves and the memento-style undo array:

```c
typedef struct Command {
    void (*execute)(ChessBoard *board, struct Command *cmd);
    void (*undo)(ChessBoard *board, struct Command *cmd);
    Move move;
    Piece capturedPiece;
} Command;
```

**Why it matters:** Each move becomes a first-class object that knows how to execute and undo itself. Enables redo functionality (navigate forward in game history) which the current system cannot do.

### 13.3 Patterns Comparison Table

| Pattern | ver1 Status | ver2 Status | Code Implemented? |
|---------|-------------|-------------|-------------------|
| Memento (Undo) | Present | Present | Yes — `S_UNDO` + `history[]` |
| Table-Driven | Present | Present | Yes — all lookup tables |
| Strategy (UI modes) | Informal | Informal | Partially — separate functions, no interface |
| Factory (Board) | Not present | Documented | No — in REFACTORING_GUIDE only |
| Observer (Events) | Not present | Documented | No — in REFACTORING_GUIDE only |
| Command (Moves) | Not present | Documented | No — in REFACTORING_GUIDE only |
| Singleton (Options) | Implicit | Implicit | Partially — `EngineOptions[1]` global, no `GetInstance()` |

---

## 14. Detailed Change Log (ver1 → ver2)

### 14.1 Structural Changes

#### File Reorganization (22 files moved)

| ver1 Location | ver2 Location | Rationale |
|--------------|---------------|-----------|
| `defs.h` | `src/core/types/defs.h` | Foundation types belong in core/types |
| `data.c` | `src/core/types/data.c` | Global data tables are type-adjacent |
| `board.c` | `src/core/board/board.c` | Board management is core logic |
| `hashkeys.c` | `src/core/board/hashkeys.c` | Hash keys are integral to board state |
| `validate.c` | `src/core/board/validate.c` | Validation is a board concern |
| `movegen.c` | `src/core/moves/movegen.c` | Move generation is core logic |
| `makemove.c` | `src/core/moves/makemove.c` | Move execution is core logic |
| `io.c` | `src/core/moves/io.c` | Move I/O formatting |
| `attack.c` | `src/core/attack/attack.c` | Attack detection is core logic |
| `bitboards.c` | `src/core/bitboards/bitboards.c` | Bitboard ops are core utilities |
| `search.c` | `src/engine/search/search.c` | Search is engine/AI layer |
| `perft.c` | `src/engine/search/perft.c` | Perft testing is search-related |
| `evaluate.c` | `src/engine/evaluation/evaluate.c` | Evaluation is engine/AI layer |
| `pvtable.c` | `src/engine/hashtable/pvtable.c` | Transposition table is engine/AI |
| `uci.c` | `src/ui/protocols/uci.c` | UCI is a UI protocol |
| `xboard.c` | `src/ui/protocols/xboard.c` | XBoard is a UI protocol |
| `gui.c` | `src/ui/sdl/gui.c` | SDL GUI is a UI implementation |
| `gui.h` | `src/ui/sdl/gui.h` | GUI header stays with GUI code |
| `init.c` | `src/utils/init.c` | Initialization is a utility |
| `misc.c` | `src/utils/misc.c` | Time/input are utilities |
| `polybook.c` | `src/openingbook/polybook.c` | Opening book is external data |
| `polykeys.c` | `src/openingbook/polykeys.c` | Polyglot keys support the book |
| `polykeys.h` | `src/openingbook/polykeys.h` | Book header stays with book code |
| `gambit.c` | `src/main/gambit.c` | Entry point gets its own folder |

#### New Folders Created

| Folder | Purpose |
|--------|---------|
| `build/obj/` | Compiled object files (mirrors `src/` tree) |
| `build/bin/` | Final executable |
| `docs/` | Documentation (REFACTORING_GUIDE.md) |
| `lib/` | SDL2 DLLs (reference only; statically linked) |
| `scripts/` | Build/setup scripts |
| `src/ui/console/` | Placeholder for console UI (empty) |

### 14.2 Build System Changes

#### ver1 Makefile
```makefile
# Simple: compile everything, link, copy DLLs
all:
    gcc -O2 -mwindows *.c -o gambit.exe -lSDL2 -lSDL2_ttf
    copy SDL2.dll .
    copy SDL2_ttf.dll .
```

#### ver2 Makefile (Key Improvements)

| Feature | Description |
|---------|-------------|
| **Source grouping** | `SOURCES_CORE` (20 files) and `SOURCES_GUI` (1 file) listed separately |
| **Object output** | `build/obj/src/core/board/board.o` mirrors source tree |
| **Static linking** | `-static` + all SDL2 dependencies linked statically (~7MB exe) |
| **Conditional GUI** | `-DENABLE_GUI` flag; GUI code compiles only when enabled |
| **Multiple targets** | `all`, `clean`, `rebuild`, `run`, `directories`, `help` |
| **Include paths** | `-Isrc/core/types` so `#include "defs.h"` works from any subfolder |

### 14.3 Code Changes

#### New Functions Added

| Function | File | Purpose |
|----------|------|---------|
| `GetBestMove()` | `search.c` | Returns best move from search without executing it — clean API for GUI |

#### Modified Functions

| Function | File | Change |
|----------|------|--------|
| `MakeMove()` | `makemove.c` | Now tracks captured pieces in `capturedWhite[]`/`capturedBlack[]` arrays |
| `TakeMove()` | `makemove.c` | Now decrements captured piece counts on undo |
| `ResetBoard()` | `board.c` | Now initializes `capturedWhiteCount`, `capturedBlackCount`, captured arrays |
| `main()` | `gambit.c` | Restructured: GUI is default mode, CLI args for uci/xboard, `#ifdef ENABLE_GUI` |

#### New Fields in `S_BOARD`

```c
int capturedWhite[16];       // Pieces captured BY white (black pieces taken)
int capturedBlack[16];       // Pieces captured BY black (white pieces taken)
int capturedWhiteCount;      // Number of pieces captured by white
int capturedBlackCount;      // Number of pieces captured by black
```

#### New GUI Features (gui.c — entirely rewritten/expanded)

| Feature | Implementation |
|---------|---------------|
| **Captured pieces panel** | Side-by-side columns (black left, white right) with `RenderCapturedPieces()` |
| **Pawn promotion dialog** | Modal dialog with 4 piece choices: `RenderPromotionDialog()`, `HandlePromotionClick()` |
| **Legal move highlighting** | Green overlay on valid squares: `CalculatePossibleMoves()`, green render in `RenderBoard()` |
| **King check highlighting** | Red square when in check: `SqAttacked()` check in `RenderBoard()` |
| **Timers** | White/black chess clocks with increment: `UpdateTimer()`, `RenderTimers()`, `ResetTimers()` |
| **Move history panel** | Scrollable move list: `AddMoveToHistory()`, `RenderMoveHistory()` |
| **Game over detection** | Checkmate, stalemate, draws: `SetGameOver()`, `RenderGameOverMessage()` |
| **Game modes** | PvP and PvE with mode toggle: `RenderGameMode()` |
| **Keyboard shortcuts** | N=new, M=mode, H=help in `RunGUI()` event loop |

### 14.4 Documentation Added

| Document | Lines | Content |
|----------|-------|---------|
| `REFACTORING_GUIDE.md` | 1,921 | Full refactoring guide: naming conventions, SOLID principles, design patterns, migration roadmap, before/after examples |
| `IMPLEMENTATION_SUMMARY.md` | 198 | Summary of GUI feature implementations |
| Doxygen headers | ~200+ lines added | File headers and function documentation across all files |

---

## 15. What Was Improved & What Remains

### 15.1 Improvements Achieved

| Area | Before (ver1) | After (ver2) |
|------|--------------|-------------|
| **File organization** | 22 files in flat directory | 6-layer modular hierarchy under `src/` |
| **Build system** | Single `gcc` command | Structured makefile with obj/bin separation, static linking |
| **Documentation** | Minimal README | 1900-line REFACTORING_GUIDE + Doxygen headers on all files |
| **GUI features** | Basic board with auto-queen promotion | Full GUI: move highlighting, check indication, promotion dialog, timers, captured pieces |
| **Engine API** | No clean API for GUI | `GetBestMove()` function provides abstraction |
| **Build portability** | Requires SDL2 DLLs | Statically linked single executable |
| **UI separation** | GUI code adjacent to engine code | `ui/` folder physically separates presentation from logic |
| **Dependency documentation** | None | Layer diagram + folder structure documents dependency flow |

### 15.2 What Still Needs Work

| Area | Current State | Needed Change | Phase |
|------|--------------|---------------|-------|
| **Naming conventions** | ver1 names preserved in engine | Apply `Module_VerbNoun` convention | Phase 4 |
| **Header split** | Single `defs.h` (1076 lines) | Split into per-module headers | Phase 4 |
| **S_BOARD decomposition** | God object (grew larger) | Split into `BoardState`, `SearchState`, `CapturedPieces` | Phase 5 |
| **Design patterns** | Documented only | Implement Strategy (UI backends), Factory (board creation) | Phase 6 |
| **Function decomposition** | Some functions >100 lines | Break `HandleMouseClick` (250 lines), `GenerateAllMoves` into helpers | Phase 5 |
| **Global variable encapsulation** | 40+ bare globals | Wrap in accessor functions or module-level structs | Phase 5 |
| **Error handling** | Custom `ASSERT` macro | Add `StatusCode` enum, proper error returns | Phase 5 |
| **Unit tests** | None | Create per-module tests | Phase 7 |

---

## 16. Lessons Learned

### 16.1 About Program Styling

- **Folder structure IS documentation.** When files are organized into `core/`, `engine/`, `ui/`, a new developer immediately understands the architecture without reading any code.
- **Naming conventions must be applied early.** We designed a comprehensive convention but only applied it to new code. Retrofitting names across 20 files is significantly harder than naming correctly from the start.
- **Doxygen headers are low-effort, high-impact.** Adding `@brief`, `@param`, `@return` to every function took modest effort but dramatically improved readability.

### 16.2 About SOLID Principles

- **SRP is the most impactful principle in C.** Without classes, the primary SRP mechanism is file and folder organization. Grouping `board.c` + `hashkeys.c` + `validate.c` under `core/board/` clearly communicates that these files share the "board management" responsibility.
- **OCP in C requires function pointers.** Unlike C++/Java where interfaces are built into the language, achieving OCP in C means defining `typedef struct { FuncPtr... }` interfaces. This is more boilerplate, but the pattern is identical.
- **ISP maps to header organization in C.** The `#include` directive is how C modules declare their dependencies. A single monolithic header violates ISP by exposing everything to everything.
- **DIP is about API design.** `GetBestMove()` is a concrete example: the GUI depends on "give me the best move" (abstraction), not on "run alpha-beta search at depth D with window [alpha, beta]" (implementation detail).

### 16.3 About Design Patterns in C

- **Design patterns are language-agnostic concepts.** Factory, Strategy, Observer, and Command all work in C using function pointers and structs. The implementation is more verbose than in OOP languages, but the architectural benefits are identical.
- **The Memento pattern was already in our code.** We had `S_UNDO` + `history[]` implementing Memento without knowing it was a "pattern." This shows that patterns describe solutions developers naturally arrive at — naming them just helps us discuss and refine them.
- **Table-Driven Design is the dominant pattern in chess engines.** Every chess engine uses lookup tables for piece values, directions, and evaluation. This is a performance-critical pattern where O(1) lookups replace O(n) computations.

### 16.4 About Using AI for Refactoring

- **AI excels at analysis and plan generation.** The initial codebase audit prompt produced a comprehensive inventory of every function, global, and struct — work that would take hours manually.
- **AI is effective for boilerplate generation.** Doxygen headers, makefile rules, and folder structure creation are repetitive tasks where AI saves significant time.
- **AI needs domain context.** Generic "refactor this code" prompts produce generic results. Prompts that reference specific chess engine concepts (bitboards, Zobrist hashing, alpha-beta pruning) produce much more relevant suggestions.
- **Human judgment is essential for trade-offs.** AI proposed renaming everything immediately. We chose to defer naming changes to preserve working code — a pragmatic decision that AI wouldn't make on its own.
- **Iterative prompting > single large prompt.** Breaking the refactoring into stages (analyze → plan → conventions → structure → code → docs) produced better results than asking "refactor everything" in one prompt.

---

## Appendix A: File Metrics Comparison

| Metric | ver1 | ver2 | Change |
|--------|------|------|--------|
| Source files (`.c`) | 20 | 20 | Same (reorganized) |
| Header files (`.h`) | 3 | 3 | Same (reorganized) |
| Total C lines | ~4,800 | ~5,200 | +400 (GUI features + docs) |
| Directories | 1 (flat) | 13 (hierarchical) | +12 |
| Documentation lines | ~200 | ~2,300 | +2,100 |
| Public functions | ~75 | ~77 | +2 (`GetBestMove`, minor) |
| Struct types | 8 | 8 | Same (fields added to `S_BOARD`) |
| Global variables | ~40 | ~40 | Same |
| Makefile targets | 1 | 6 | +5 |
| External DLL dependencies | 2 (SDL2, SDL2_ttf) | 0 (static) | -2 |

## Appendix B: Dependency Direction Verification

To verify our layered architecture enforces correct dependency flow, here's which modules each layer calls:

| Source File | Calls Functions From | Layer |
|-------------|---------------------|-------|
| `gui.c` | `MakeMove`, `TakeMove`, `GenerateAllMoves`, `ParseFen`, `SqAttacked`, `GetBestMove`, `EvalPosition`, `ParseMove`, `PrMove`, `ResetBoard`, `CheckBoard`, `InitPolyBook`, `GetBookMove`, `SearchPosition` | UI → Engine + Core |
| `uci.c` | `ParseFen`, `MakeMove`, `ParseMove`, `SearchPosition`, `ResetBoard`, `InitHashTable`, `ClearHashTable` | UI → Engine + Core |
| `xboard.c` | `ParseFen`, `MakeMove`, `ParseMove`, `SearchPosition`, `ThreeFoldRep`, `DrawMaterial`, `PrintBoard`, `GenerateAllMoves`, `ResetBoard`, `InitHashTable`, `ClearHashTable`, `GetBookMove` | UI → Engine + Core |
| `search.c` | `EvalPosition`, `GenerateAllMoves`, `GenerateAllCaps`, `MakeMove`, `TakeMove`, `MakeNullMove`, `TakeNullMove`, `SqAttacked`, `ProbeHashEntry`, `StoreHashEntry`, `ProbePvMove`, `GetPvLine`, `ReadInput` | Engine → Core + Utils |
| `evaluate.c` | (none — leaf module, reads from `S_BOARD`) | Engine (standalone) |
| `movegen.c` | (none — reads from `S_BOARD`, uses macros) | Core (standalone) |
| `board.c` | `GeneratePosKey` | Core → Core |
| `attack.c` | (none — reads from `S_BOARD`) | Core (standalone) |

**Conclusion:** Dependencies flow downward (UI → Engine → Core) with no upward calls. This validates the layered architecture.
