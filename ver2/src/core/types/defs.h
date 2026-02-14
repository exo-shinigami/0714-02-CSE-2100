/**
 * @file defs.h
 * @brief Core definitions, data structures, and function declarations for Gambit Chess Engine
 * 
 * This header file contains all the fundamental type definitions, constants, macros,
 * and function prototypes used throughout the Gambit chess engine. It serves as the
 * central reference point for the entire codebase.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 * @version 1.1
 * 
 * Key Components:
 * - Board representation (120-square mailbox + bitboards)
 * - Move encoding and manipulation
 * - Search data structures
 * - Hash table (transposition table)
 * - Function declarations for all modules
 */

#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"
#include "stdio.h"

// #define DEBUG   // Enable for debugging assertions and extra checks

/* ===========================================================================
 * CONSTANTS AND CONFIGURATION
 * ===========================================================================
 */

#define MAX_HASH 1024  // Maximum hash table size in MB

/**
 * ASSERT macro for debugging
 * - In DEBUG mode: Checks condition and prints detailed error if false
 * - In RELEASE mode: Disabled for performance
 */
#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

/**
 * @typedef U64
 * @brief Unsigned 64-bit integer used for bitboards and hash keys
 */
typedef unsigned long long U64;

#define NAME "Gambit 1.1"  // Engine name and version

/**
 * Board representation uses 120-square mailbox (10x12) to simplify bounds checking
 * This includes padding around the standard 8x8 board
 */
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048        // Maximum moves in a single game
#define MAXPOSITIONMOVES 256     // Maximum legal moves in any position
#define MAXDEPTH 64              // Maximum search depth

/** Standard chess starting position in FEN notation */
#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define INFINITE 30000           // Infinite score for search bounds
#define ISMATE (INFINITE - MAXDEPTH)  // Score threshold for mate detection

/* ===========================================================================
 * ENUMERATIONS
 * ===========================================================================
 */

/**
 * Piece types encoding
 * - EMPTY (0): No piece
 * - wP-wK: White pieces (Pawn, Knight, Bishop, Rook, Queen, King)
 * - bP-bK: Black pieces (Pawn, Knight, Bishop, Rook, Queen, King)
 */
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };

/** File names (A-H) and FILE_NONE for special cases */
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };

/** Rank numbers (1-8) and RANK_NONE for special cases */
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

/**
 * Colors encoding
 * - WHITE (0): White pieces
 * - BLACK (1): Black pieces
 * - BOTH (2): Used for combined bitboards
 */
enum { WHITE, BLACK, BOTH };

/** Engine operating modes */
enum { UCIMODE, XBOARDMODE, CONSOLEMODE };

/**
 * Square indices for 120-square mailbox representation
 * - Valid squares: A1-H8 (21-98, excluding padding)
 * - NO_SQ: No square (used for en passant when not applicable)
 * - OFFBOARD: Padding squares outside the board
 */
enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

/** Boolean values */
enum { FALSE, TRUE };

/**
 * Castle permission flags (bit flags, can be combined)
 * - WKCA (1): White king-side castle
 * - WQCA (2): White queen-side castle
 * - BKCA (4): Black king-side castle
 * - BQCA (8): Black queen-side castle
 */
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

/* ===========================================================================
 * DATA STRUCTURES
 * ===========================================================================
 */

/**
 * @struct S_MOVE
 * @brief Represents a move with its associated score (for move ordering)
 * @field move - Encoded move (see move encoding documentation below)
 * @field score - Score for move ordering in search
 */
typedef struct {
	int move;
	int score;
} S_MOVE;

/**
 * @struct S_MOVELIST
 * @brief List of moves generated for a position
 * @field moves - Array of moves with scores
 * @field count - Number of moves in the list
 */
typedef struct {
	S_MOVE moves[MAXPOSITIONMOVES];
	int count;
} S_MOVELIST;

/**
 * Hash table entry flags
 * - HFNONE: No flag
 * - HFALPHA: Alpha bound (upper bound on true value)
 * - HFBETA: Beta bound (lower bound on true value)
 * - HFEXACT: Exact score
 */
enum {  HFNONE, HFALPHA, HFBETA, HFEXACT};

/**
 * @struct S_HASHENTRY
 * @brief Transposition table entry
 * @field posKey - Zobrist hash key for position verification
 * @field move - Best move found for this position
 * @field score - Evaluation score
 * @field depth - Search depth at which this entry was created
 * @field flags - Entry type (exact, alpha, or beta bound)
 */
typedef struct {
	U64 posKey;
	int move;
	int score;
	int depth;
	int flags;
} S_HASHENTRY;

/**
 * @struct S_HASHTABLE
 * @brief Transposition table for storing previously searched positions
 * @field pTable - Pointer to hash table array
 * @field numEntries - Number of entries in table
 * @field newWrite - Count of new entries written
 * @field overWrite - Count of entries overwritten
 * @field hit - Count of successful lookups
 * @field cut - Count of cutoffs from hash table
 */
typedef struct {
	S_HASHENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} S_HASHTABLE;

/**
 * @struct S_UNDO
 * @brief Information needed to undo a move
 * @field move - The move that was made
 * @field castlePerm - Castle permissions before the move
 * @field enPas - En passant square before the move
 * @field fiftyMove - Fifty move counter before the move
 * @field posKey - Position hash key before the move
 */
typedef struct {

	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;

} S_UNDO;

/**
 * @struct S_BOARD
 * @brief Complete board representation and game state
 * 
 * Uses hybrid representation:
 * - 120-square mailbox for fast piece lookup
 * - Bitboards for fast pawn operations
 * - Piece lists for iteration
 * 
 * @field pieces - 120-square mailbox array
 * @field pawns - Bitboards for pawns [WHITE, BLACK, BOTH]
 * @field KingSq - King square positions [WHITE, BLACK]
 * @field side - Side to move (WHITE or BLACK)
 * @field enPas - En passant target square (NO_SQ if not available)
 * @field fiftyMove - Half-move clock for fifty-move rule
 * @field ply - Current search ply (distance from root)
 * @field hisPly - Total plies in game history
 * @field castlePerm - Castle permission flags
 * @field posKey - Zobrist hash of current position
 * @field pceNum - Count of each piece type on board
 * @field bigPce - Count of non-pawn pieces [WHITE, BLACK]
 * @field majPce - Count of rooks and queens [WHITE, BLACK]
 * @field minPce - Count of bishops and knights [WHITE, BLACK]
 * @field material - Material score [WHITE, BLACK]
 * @field history - Array of undo information for previous moves
 * @field pList - Piece lists organized by type
 * @field HashTable - Transposition table
 * @field PvArray - Principal variation array
 * @field searchHistory - History heuristic scores
 * @field searchKillers - Killer move heuristic
 * @field capturedWhite - Array of captured white pieces (for GUI display)
 * @field capturedBlack - Array of captured black pieces (for GUI display)
 * @field capturedWhiteCount - Count of captured white pieces
 * @field capturedBlackCount - Count of captured black pieces
 */
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

	// piece list
	int pList[13][10];

	S_HASHTABLE HashTable[1];
	int PvArray[MAXDEPTH];

	int searchHistory[13][BRD_SQ_NUM];
	int searchKillers[2][MAXDEPTH];
	
	// Captured pieces tracking
	int capturedWhite[16];  // Array to store captured white pieces
	int capturedBlack[16];  // Array to store captured black pieces
	int capturedWhiteCount; // Count of captured white pieces
	int capturedBlackCount; // Count of captured black pieces

} S_BOARD;

/**
 * @struct S_SEARCHINFO
 * @brief Search control and statistics
 * @field starttime - Time when search started
 * @field stoptime - Time when search should stop
 * @field depth - Maximum search depth
 * @field timeset - Whether time control is set
 * @field movestogo - Moves until next time control
 * @field nodes - Number of nodes searched
 * @field quit - Flag to stop search and exit
 * @field stopped - Flag indicating search was stopped
 * @field fh - First move fail high count (for move ordering statistics)
 * @field fhf - First move fail high first count
 * @field nullCut - Null move cutoffs count
 * @field GAME_MODE - Current game mode (UCI, XBoard, Console)
 * @field POST_THINKING - Whether to post thinking output
 */
typedef struct {

	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;

	long nodes;

	int quit;
	int stopped;

	float fh;
	float fhf;
	int nullCut;

	int GAME_MODE;
	int POST_THINKING;

} S_SEARCHINFO;

/**
 * @struct S_OPTIONS
 * @brief Engine configuration options
 * @field UseBook - Whether to use opening book
 */
typedef struct {
	int UseBook;
} S_OPTIONS;


/* GAME MOVE */

/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

/* ===========================================================================
 * MOVE ENCODING MACROS
 * 
 * Moves are encoded in a single integer using bit manipulation:
 * - Bits 0-6:   From square (0-127)
 * - Bits 7-13:  To square (0-127)
 * - Bits 14-17: Captured piece type (0-15)
 * - Bit 18:     En passant capture flag
 * - Bit 19:     Pawn start (double push) flag
 * - Bits 20-23: Promoted piece type (0-15)
 * - Bit 24:     Castle flag
 * ===========================================================================
 */

#define FROMSQ(m) ((m) & 0x7F)         // Extract source square from move
#define TOSQ(m) (((m)>>7) & 0x7F)      // Extract destination square from move
#define CAPTURED(m) (((m)>>14) & 0xF)  // Extract captured piece from move
#define PROMOTED(m) (((m)>>20) & 0xF)  // Extract promoted piece from move

#define MFLAGEP 0x40000     // En passant capture flag
#define MFLAGPS 0x80000     // Pawn start (double push) flag
#define MFLAGCA 0x1000000   // Castle flag

#define MFLAGCAP 0x7C000    // Mask for capture flag
#define MFLAGPROM 0xF00000  // Mask for promotion flag

#define NOMOVE 0            // Represents no move/invalid move


/* MACROS */

/* ===========================================================================
 * UTILITY MACROS
 * ===========================================================================
 */

/** Convert file and rank to 120-square index */
#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )

/** Convert between 120-square and 64-square representations */
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])

/** Bitboard operations */
#define POP(b) PopBit(b)                 // Pop least significant bit
#define CNT(b) CountBits(b)              // Count bits in bitboard
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])  // Clear bit at square
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])    // Set bit at square

/** Piece type queries */
#define IsBQ(p) (PieceBishopQueen[(p)])  // Is piece a bishop or queen?
#define IsRQ(p) (PieceRookQueen[(p)])    // Is piece a rook or queen?
#define IsKn(p) (PieceKnight[(p)])       // Is piece a knight?
#define IsKi(p) (PieceKing[(p)])         // Is piece a king?

/** Mirror square for evaluation symmetry */
#define MIRROR64(sq) (Mirror64[(sq)])

/* ===========================================================================
 * GLOBAL VARIABLES
 * ===========================================================================
 */

/* GLOBALS */

// Square conversion tables
extern int Sq120ToSq64[BRD_SQ_NUM];  // Map 120-square to 64-square index
extern int Sq64ToSq120[64];          // Map 64-square to 120-square index

// Bitboard masks
extern U64 SetMask[64];    // Masks to set a bit at each square
extern U64 ClearMask[64];  // Masks to clear a bit at each square

// Zobrist hashing keys
extern U64 PieceKeys[13][120];  // Hash keys for pieces at squares
extern U64 SideKey;          // Hash key for side to move
extern U64 CastleKeys[16];   // Hash keys for castle permissions

// Character representations
extern char PceChar[];   // Character for each piece type
extern char SideChar[];  // Character for each side
extern char RankChar[];  // Character for each rank
extern char FileChar[];  // Character for each file

// Piece properties
extern int PieceBig[13];        // Is piece a "big" piece (not pawn)?
extern int PieceMaj[13];        // Is piece major (rook/queen)?
extern int PieceMin[13];        // Is piece minor (bishop/knight)?
extern int PieceVal[13];        // Material value of each piece
extern int PieceCol[13];        // Color of each piece
extern int PiecePawn[13];       // Is piece a pawn?

// Board properties
extern int FilesBrd[BRD_SQ_NUM];  // File of each square
extern int RanksBrd[BRD_SQ_NUM];  // Rank of each square

// Piece movement properties
extern int PieceKnight[13];      // Is piece a knight?
extern int PieceKing[13];        // Is piece a king?
extern int PieceRookQueen[13];   // Does piece move like rook/queen?
extern int PieceBishopQueen[13]; // Does piece move like bishop/queen?
extern int PieceSlides[13];      // Is piece a sliding piece?

// Evaluation helper arrays
extern int Mirror64[64];  // Mirror table for position evaluation

// Bitboard masks for evaluation
extern U64 FileBBMask[8];         // Bitboard for each file
extern U64 RankBBMask[8];         // Bitboard for each rank
extern U64 BlackPassedMask[64];   // Passed pawn masks for black
extern U64 WhitePassedMask[64];   // Passed pawn masks for white
extern U64 IsolatedMask[64];      // Isolated pawn masks

// Engine options
extern S_OPTIONS EngineOptions[1];

/* ===========================================================================
 * FUNCTION DECLARATIONS
 * ===========================================================================
 */

/* FUNCTIONS */

/* ---------------------------------------------------------------------------
 * INITIALIZATION (init.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Initialize all global arrays and data structures
 * 
 * This function must be called once before using the engine. It initializes:
 * - Square conversion tables
 * - Bitboard masks
 * - Zobrist hash keys
 * - Evaluation masks
 * - Move generation tables
 */
extern void AllInit();

/* ---------------------------------------------------------------------------
 * BITBOARD OPERATIONS (bitboards.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Print a bitboard as an 8x8 grid for debugging
 * @param bb Bitboard to print
 */
extern void PrintBitBoard(U64 bb);

/**
 * @brief Remove and return the index of the least significant bit
 * @param bb Pointer to bitboard
 * @return Index of the LSB (0-63), or -1 if bitboard is empty
 */
extern int PopBit(U64 *bb);

/**
 * @brief Count the number of set bits in a bitboard
 * @param b Bitboard to count
 * @return Number of set bits
 */
extern int CountBits(U64 b);

/* ---------------------------------------------------------------------------
 * HASH KEYS (hashkeys.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Generate Zobrist hash key for current board position
 * @param pos Board position
 * @return 64-bit hash key uniquely identifying the position
 */
extern U64 GeneratePosKey(const S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * BOARD OPERATIONS (board.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Reset board to empty state
 * @param pos Board to reset
 */
extern void ResetBoard(S_BOARD *pos);

/**
 * @brief Parse FEN string and set up board position
 * @param fen FEN string to parse
 * @param pos Board to set up
 * @return TRUE if successful, FALSE if FEN is invalid
 */
extern int ParseFen(char *fen, S_BOARD *pos);

/**
 * @brief Print board position to console in ASCII art
 * @param pos Board to print
 */
extern void PrintBoard(const S_BOARD *pos);

/**
 * @brief Update piece lists and material counts from board array
 * @param pos Board to update
 * 
 * This function should be called after directly modifying the pieces array
 * to keep derived data structures in sync.
 */
extern void UpdateListsMaterial(S_BOARD *pos);

/**
 * @brief Validate board consistency for debugging
 * @param pos Board to check
 * @return TRUE if board is valid, FALSE otherwise
 * 
 * Verifies:
 * - Piece lists match board array
 * - Bitboards match board array
 * - Material counts are correct
 * - Position hash is correct
 */
extern int CheckBoard(const S_BOARD *pos);

/**
 * @brief Mirror board position vertically (for evaluation testing)
 * @param pos Board to mirror
 */
extern void MirrorBoard(S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * ATTACK DETECTION (attack.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Check if a square is attacked by a given side
 * @param sq Square to check (120-square index)
 * @param side Color of attacking side (WHITE or BLACK)
 * @param pos Board position
 * @return TRUE if square is attacked, FALSE otherwise
 * 
 * Used for:
 * - Check detection
 * - Castle legality
 * - Move validation
 */
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * INPUT/OUTPUT (io.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Convert move to string in algebraic notation
 * @param move Move to convert
 * @return String representation (e.g., "e2e4", "e7e8q")
 * 
 * Note: Returns pointer to static buffer, not thread-safe
 */
extern char *PrMove(const int move);

/**
 * @brief Convert square to string in algebraic notation
 * @param sq Square index (120-square)
 * @return String representation (e.g., "e4", "a1")
 * 
 * Note: Returns pointer to static buffer, not thread-safe
 */
extern char *PrSq(const int sq);

/**
 * @brief Print all moves in a move list
 * @param list Move list to print
 */
extern void PrintMoveList(const S_MOVELIST *list);

/**
 * @brief Parse string to move (e.g., "e2e4" to encoded move)
 * @param ptrChar String to parse
 * @param pos Current board position
 * @return Encoded move if valid, NOMOVE otherwise
 */
extern int ParseMove(char *ptrChar, S_BOARD *pos);


/* ---------------------------------------------------------------------------
 * VALIDATION (validate.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Check if square is on the board
 * @param sq Square index (120-square)
 * @return TRUE if on board, FALSE if offboard or invalid
 */
extern int SqOnBoard(const int sq);

/**
 * @brief Check if side value is valid
 * @param side Side to check
 * @return TRUE if WHITE or BLACK, FALSE otherwise
 */
extern int SideValid(const int side);

/**
 * @brief Check if file or rank is valid
 * @param fr File or rank value
 * @return TRUE if valid (0-7), FALSE otherwise
 */
extern int FileRankValid(const int fr);

/**
 * @brief Check if piece is valid or EMPTY
 * @param pce Piece value
 * @return TRUE if valid piece or EMPTY, FALSE otherwise
 */
extern int PieceValidEmpty(const int pce);

/**
 * @brief Check if piece is valid (not EMPTY)
 * @param pce Piece value
 * @return TRUE if valid piece, FALSE otherwise
 */
extern int PieceValid(const int pce);

/**
 * @brief Testing function for evaluation symmetry
 * @param pos Board position
 */
extern void MirrorEvalTest(S_BOARD *pos);

/**
 * @brief Check if square index is in 120-square format
 * @param sq Square index
 * @return TRUE if valid 120-square index, FALSE otherwise
 */
extern int SqIs120(const int sq);

/**
 * @brief Check if piece is valid, EMPTY, or OFFBOARD
 * @param pce Piece value
 * @return TRUE if valid, FALSE otherwise
 */
extern int PceValidEmptyOffbrd(const int pce);

/**
 * @brief Validate move list integrity
 * @param list Move list to check
 * @param pos Board position
 * @return TRUE if valid, FALSE otherwise
 */
extern int MoveListOk(const S_MOVELIST *list,  const S_BOARD *pos);

/**
 * @brief Debug function for analysis testing
 * @param pos Board position
 * @param info Search information
 */
extern void DebugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info);

/* ---------------------------------------------------------------------------
 * MOVE GENERATION (movegen.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Generate all pseudo-legal moves for current position
 * @param pos Board position
 * @param list Output move list
 * 
 * Generates all moves including those that leave king in check.
 * Moves must be validated with MakeMove to ensure legality.
 */
extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);

/**
 * @brief Generate all capture moves only
 * @param pos Board position
 * @param list Output move list
 * 
 * Used in quiescence search to consider only tactical moves.
 */
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);

/**
 * @brief Check if a move exists in current position
 * @param pos Board position
 * @param move Move to check
 * @return TRUE if move is legal, FALSE otherwise
 */
extern int MoveExists(S_BOARD *pos, const int move);

/**
 * @brief Initialize MVV-LVA (Most Valuable Victim - Least Valuable Attacker) table
 * 
 * This table is used for move ordering to search captures in optimal order.
 */
extern void InitMvvLva();

/* ---------------------------------------------------------------------------
 * MAKE/UNMAKE MOVES (makemove.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Make a move on the board
 * @param pos Board position (modified)
 * @param move Move to make
 * @return TRUE if move is legal (king not in check after move), FALSE otherwise
 * 
 * Updates board state, hash key, and all derived information.
 * If move is illegal, board state is unchanged.
 */
extern int MakeMove(S_BOARD *pos, int move);

/**
 * @brief Unmake the last move (rollback to previous position)
 * @param pos Board position (modified)
 * 
 * Restores board to state before last MakeMove call.
 * Must be called in reverse order of MakeMove.
 */
extern void TakeMove(S_BOARD *pos);

/**
 * @brief Make a null move (pass without moving)
 * @param pos Board position (modified)
 * 
 * Used for null move pruning in search.
 * Only changes side to move and updates hash.
 */
extern void MakeNullMove(S_BOARD *pos);

/**
 * @brief Unmake a null move
 * @param pos Board position (modified)
 */
extern void TakeNullMove(S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * PERFT (perft.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Performance test - count all leaf nodes at given depth
 * @param depth Depth to search
 * @param pos Board position
 * 
 * Used for debugging move generation and make/unmake functions.
 * Prints node counts for each root move.
 */
extern void PerftTest(int depth, S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * SEARCH (search.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Search for best move using iterative deepening
 * @param pos Board position
 * @param info Search control information
 * 
 * Main search function using alpha-beta with various enhancements:
 * - Iterative deepening
 * - Principal variation search
 * - Null move pruning
 * - Transposition table
 * - Quiescence search
 */
extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info);

/**
 * @brief Get best move for current position (wrapper for SearchPosition)
 * @param pos Board position
 * @param info Search control information
 * @return Best move found
 */
extern int GetBestMove(S_BOARD *pos, S_SEARCHINFO *info);

/* ---------------------------------------------------------------------------
 * MISC UTILITIES (misc.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Get current time in milliseconds
 * @return Time in milliseconds
 */
extern int GetTimeMs();

/**
 * @brief Check for user input and handle GUI/console commands
 * @param info Search information (may set quit or stopped flags)
 */
extern void ReadInput(S_SEARCHINFO *info);

/* ---------------------------------------------------------------------------
 * TRANSPOSITION TABLE (pvtable.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Initialize hash table with specified size
 * @param table Hash table to initialize
 * @param MB Size in megabytes
 * 
 * Allocates memory for hash table. Call before first use.
 */
extern void InitHashTable(S_HASHTABLE *table, const int MB);

/**
 * @brief Store position in hash table
 * @param pos Board position
 * @param move Best move for this position
 * @param score Score for this position
 * @param flags Entry type (HFEXACT, HFALPHA, or HFBETA)
 * @param depth Depth at which position was searched
 */
extern void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);

/**
 * @brief Probe hash table for position
 * @param pos Board position
 * @param move Output parameter for best move
 * @param score Output parameter for score
 * @param alpha Alpha bound
 * @param beta Beta bound
 * @param depth Current search depth
 * @return TRUE if usable entry found, FALSE otherwise
 */
extern int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth);

/**
 * @brief Get principal variation move from hash table
 * @param pos Board position
 * @return Best move from hash table, or NOMOVE if not found
 */
extern int ProbePvMove(const S_BOARD *pos);

/**
 * @brief Extract principal variation line from hash table
 * @param depth Maximum depth
 * @param pos Board position
 * @return Number of moves in PV line
 */
extern int GetPvLine(const int depth, S_BOARD *pos);

/**
 * @brief Clear all entries in hash table
 * @param table Hash table to clear
 */
extern void ClearHashTable(S_HASHTABLE *table);

/* ---------------------------------------------------------------------------
 * EVALUATION (evaluate.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Evaluate board position from current side's perspective
 * @param pos Board position
 * @return Evaluation score in centipawns (positive = current side better)
 * 
 * Considers:
 * - Material balance
 * - Piece-square tables
 * - Pawn structure
 * - Piece mobility
 * - King safety
 */
extern int EvalPosition(const S_BOARD *pos);

/**
 * @brief Test evaluation symmetry by mirroring position
 * @param pos Board position
 */
extern void MirrorEvalTest(S_BOARD *pos) ;

/* ---------------------------------------------------------------------------
 * UCI PROTOCOL (uci.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Main loop for UCI (Universal Chess Interface) protocol
 * @param pos Board position
 * @param info Search information
 * 
 * Handles UCI commands from GUI:
 * - uci, isready, ucinewgame
 * - position, go, stop
 * - setoption
 */
extern void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info);

/* ---------------------------------------------------------------------------
 * XBOARD PROTOCOL (xboard.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Main loop for XBoard/WinBoard protocol
 * @param pos Board position
 * @param info Search information
 */
extern void XBoard_Loop(S_BOARD *pos, S_SEARCHINFO *info);

/**
 * @brief Main loop for console mode (human play)
 * @param pos Board position
 * @param info Search information
 */
extern void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info);

/**
 * @brief Check if game is over (checkmate, stalemate, draw)
 * @param pos Board position
 * @return Result code (0 = game continues, non-zero = game over)
 */
extern int checkresult(S_BOARD *pos);

/**
 * @brief Check if position is drawn by insufficient material
 * @param pos Board position
 * @return TRUE if draw, FALSE otherwise
 * 
 * Recognizes:
 * - K vs K
 * - K+minor vs K
 * - K+B vs K+B (same color bishops)
 */
extern int DrawMaterial(const S_BOARD *pos);

/**
 * @brief Check if position has occurred three times (draw by repetition)
 * @param pos Board position
 * @return TRUE if threefold repetition, FALSE otherwise
 */
extern int ThreeFoldRep(const S_BOARD *pos);

/* ---------------------------------------------------------------------------
 * POLYGLOT OPENING BOOK (polybook.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Get move from opening book for current position
 * @param board Board position
 * @return Move from book, or NOMOVE if position not in book
 */
extern int GetBookMove(S_BOARD *board);

/**
 * @brief Clean up opening book resources
 */
extern void CleanPolyBook();

/**
 * @brief Initialize opening book
 * 
 * Loads opening book file if available.
 */
extern void InitPolyBook() ;

#endif








































