/**
 * @file types_definitions.h
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

#define CHESS_MAX_HASH 1024  // Maximum hash table size in MB

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
#define CHESS_BOARD_SQUARE_NUM 120

#define CHESS_MAX_GAME_MOVES 2048        // Maximum moves in a single game
#define CHESS_MAX_POSITION_MOVES 256     // Maximum legal moves in any position
#define CHESS_MAX_SEARCH_DEPTH 64              // Maximum search depth

/** Standard chess starting position in FEN notation */
#define CHESS_START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define CHESS_INFINITE 30000           // Infinite score for search bounds
#define CHESS_IS_MATE (CHESS_INFINITE - CHESS_MAX_SEARCH_DEPTH)  // Score threshold for mate detection

/* ===========================================================================
 * ENUMERATIONS
 * ===========================================================================
 */

/**
 * Piece types encoding
 * - EMPTY (0): No piece
 * - PIECE_TYPE_WHITE_PAWN-PIECE_TYPE_WHITE_KING: White pieces (Pawn, Knight, Bishop, Rook, Queen, King)
 * - PIECE_TYPE_BLACK_PAWN-PIECE_TYPE_BLACK_KING: Black pieces (Pawn, Knight, Bishop, Rook, Queen, King)
 */
enum { EMPTY, PIECE_TYPE_WHITE_PAWN, PIECE_TYPE_WHITE_KNIGHT, PIECE_TYPE_WHITE_BISHOP, PIECE_TYPE_WHITE_ROOK, PIECE_TYPE_WHITE_QUEEN, PIECE_TYPE_WHITE_KING, PIECE_TYPE_BLACK_PAWN, PIECE_TYPE_BLACK_KNIGHT, PIECE_TYPE_BLACK_BISHOP, PIECE_TYPE_BLACK_ROOK, PIECE_TYPE_BLACK_QUEEN, PIECE_TYPE_BLACK_KING  };

/** File names (A-H) and FILE_TYPE_NONE for special cases */
enum { FILE_TYPE_A, FILE_TYPE_B, FILE_TYPE_C, FILE_TYPE_D, FILE_TYPE_E, FILE_TYPE_F, FILE_TYPE_G, FILE_TYPE_H, FILE_TYPE_NONE };

/** Rank numbers (1-8) and RANK_TYPE_NONE for special cases */
enum { RANK_TYPE_1, RANK_TYPE_2, RANK_TYPE_3, RANK_TYPE_4, RANK_TYPE_5, RANK_TYPE_6, RANK_TYPE_7, RANK_TYPE_8, RANK_TYPE_NONE };

/**
 * Colors encoding
 * - COLOR_TYPE_WHITE (0): White pieces
 * - COLOR_TYPE_BLACK (1): Black pieces
 * - COLOR_TYPE_BOTH (2): Used for combined bitboards
 */
enum { COLOR_TYPE_WHITE, COLOR_TYPE_BLACK, COLOR_TYPE_BOTH };

/** Engine operating modes */
enum { MODE_TYPE_UCI, MODE_TYPE_XBOARD, MODE_TYPE_CONSOLE };

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
enum { BOOL_TYPE_FALSE, BOOL_TYPE_TRUE };

/**
 * Castle permission flags (bit flags, can be combined)
 * - CASTLE_TYPE_WKCA (1): White king-side castle
 * - CASTLE_TYPE_WQCA (2): White queen-side castle
 * - CASTLE_TYPE_BKCA (4): Black king-side castle
 * - CASTLE_TYPE_BQCA (8): Black queen-side castle
 */
enum { CASTLE_TYPE_WKCA = 1, CASTLE_TYPE_WQCA = 2, CASTLE_TYPE_BKCA = 4, CASTLE_TYPE_BQCA = 8 };

/* ===========================================================================
 * DATA STRUCTURES
 * ===========================================================================
 */

/**
 * @struct Move
 * @brief Represents a move with its associated score (for move ordering)
 * @field move - Encoded move (see move encoding documentation below)
 * @field score - Score for move ordering in search
 */
typedef struct {
	int move;
	int score;
} Move;

/**
 * @struct MoveList
 * @brief List of moves generated for a position
 * @field moves - Array of moves with scores
 * @field count - Number of moves in the list
 */
typedef struct {
	Move moves[CHESS_MAX_POSITION_MOVES];
	int count;
} MoveList;

/**
 * Hash table entry flags
 * - HFNONE: No flag
 * - HFALPHA: Alpha bound (upper bound on true value)
 * - HFBETA: Beta bound (lower bound on true value)
 * - HFEXACT: Exact score
 */
enum {  HFNONE, HFALPHA, HFBETA, HFEXACT};

/**
 * @struct HashEntry
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
} HashEntry;

/**
 * @struct HashTable
 * @brief Transposition table for storing previously searched positions
 * @field pTable - Pointer to hash table array
 * @field numEntries - Number of entries in table
 * @field newWrite - Count of new entries written
 * @field overWrite - Count of entries overwritten
 * @field hit - Count of successful lookups
 * @field cut - Count of cutoffs from hash table
 */
typedef struct {
	HashEntry *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} HashTable;

/**
 * @struct UndoMove
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

} UndoMove;

/**
 * @struct ChessBoard
 * @brief Complete board representation and game state
 * 
 * Uses hybrid representation:
 * - 120-square mailbox for fast piece lookup
 * - Bitboards for fast pawn operations
 * - Piece lists for iteration
 * 
 * @field pieces - 120-square mailbox array
 * @field pawns - Bitboards for pawns [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK, COLOR_TYPE_BOTH]
 * @field KingSq - King square positions [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK]
 * @field side - Side to move (COLOR_TYPE_WHITE or COLOR_TYPE_BLACK)
 * @field enPas - En passant target square (NO_SQ if not available)
 * @field fiftyMove - Half-move clock for fifty-move rule
 * @field ply - Current search ply (distance from root)
 * @field hisPly - Total plies in game history
 * @field castlePerm - Castle permission flags
 * @field posKey - Zobrist hash of current position
 * @field pieceCount - Count of each piece type on board
 * @field bigPce - Count of non-pawn pieces [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK]
 * @field majPce - Count of rooks and queens [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK]
 * @field minPce - Count of bishops and knights [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK]
 * @field material - Material score [COLOR_TYPE_WHITE, COLOR_TYPE_BLACK]
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

	int pieces[CHESS_BOARD_SQUARE_NUM];
	U64 pawns[3];

	int KingSq[2];

	int side;
	int enPas;
	int fiftyMove;

	int ply;
	int hisPly;

	int castlePerm;

	U64 posKey;

	int pieceCount[13];
	int bigPce[2];
	int majPce[2];
	int minPce[2];
	int material[2];

	UndoMove history[CHESS_MAX_GAME_MOVES];

	// piece list
	int pList[13][10];

	HashTable HashTable[1];
	int PvArray[CHESS_MAX_SEARCH_DEPTH];

	int searchHistory[13][CHESS_BOARD_SQUARE_NUM];
	int searchKillers[2][CHESS_MAX_SEARCH_DEPTH];
	
	// Captured pieces tracking
	int capturedWhite[16];  // Array to store captured white pieces
	int capturedBlack[16];  // Array to store captured black pieces
	int capturedWhiteCount; // Count of captured white pieces
	int capturedBlackCount; // Count of captured black pieces

} ChessBoard;

/**
 * @struct SearchInfo
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

} SearchInfo;

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

#define MOVE_GET_FROM_SQUARE(m) ((m) & 0x7F)         // Extract source square from move
#define MOVE_GET_TO_SQUARE(m) (((m)>>7) & 0x7F)      // Extract destination square from move
#define MOVE_GET_CAPTURED(m) (((m)>>14) & 0xF)  // Extract captured piece from move
#define MOVE_GET_PROMOTED(m) (((m)>>20) & 0xF)  // Extract promoted piece from move

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
#define FILE_RANK_TO_SQUARE(f,r) ( (21 + (f) ) + ( (r) * 10 ) )

/** Convert between 120-square and 64-square representations */
#define SQUARE_120_TO_64(sq120) (g_square120To64[(sq120)])
#define SQUARE_64_TO_120(sq64) (g_square64To120[(sq64)])

/** Bitboard operations */
#define BITBOARD_POP(b) Bitboard_PopBit(b)                 // Pop least significant bit
#define BITBOARD_COUNT(b) Bitboard_CountBits(b)              // Count bits in bitboard
#define BITBOARD_CLEAR_BIT(bb,squareIndex) ((bb) &= g_bitClearMask[(squareIndex)])  // Clear bit at square
#define BITBOARD_SET_BIT(bb,squareIndex) ((bb) |= g_bitSetMask[(squareIndex)])    // Set bit at square

/** Piece type queries */
#define PIECE_IS_BISHOP_QUEEN(p) (g_pieceBishopQueen[(p)])  // Is piece a bishop or queen?
#define PIECE_IS_ROOK_QUEEN(p) (g_pieceRookQueen[(p)])    // Is piece a rook or queen?
#define PIECE_IS_KNIGHT(p) (g_pieceKnight[(p)])       // Is piece a knight?
#define PIECE_IS_KING(p) (g_pieceKing[(p)])         // Is piece a king?

/** Mirror square for evaluation symmetry */
#define SQUARE_MIRROR_64(squareIndex) (Mirror64[(squareIndex)])

/* ===========================================================================
 * GLOBAL VARIABLES
 * ===========================================================================
 */

/* GLOBALS */

// Square conversion tables
extern int g_square120To64[CHESS_BOARD_SQUARE_NUM];  // Map 120-square to 64-square index
extern int g_square64To120[64];          // Map 64-square to 120-square index

// Bitboard masks
extern U64 g_bitSetMask[64];    // Masks to set a bit at each square
extern U64 g_bitClearMask[64];  // Masks to clear a bit at each square

// Zobrist hashing keys
extern U64 g_pieceKeys[13][120];  // Hash keys for pieces at squares
extern U64 g_sideKey;          // Hash key for side to move
extern U64 g_castleKeys[16];   // Hash keys for castle permissions

// Character representations
extern char PceChar[];   // Character for each piece type
extern char SideChar[];  // Character for each side
extern char RankChar[];  // Character for each rank
extern char FileChar[];  // Character for each file

// Piece properties
extern int PieceBig[13];        // Is piece a "big" piece (not pawn)?
extern int PieceMaj[13];        // Is piece major (rook/queen)?
extern int PieceMin[13];        // Is piece minor (bishop/knight)?
extern int g_pieceVal[13];        // Material value of each piece
extern int g_pieceCol[13];        // Color of each piece
extern int g_piecePawn[13];       // Is piece a pawn?

// Board properties
extern int g_filesBoard[CHESS_BOARD_SQUARE_NUM];  // File of each square
extern int g_ranksBoard[CHESS_BOARD_SQUARE_NUM];  // Rank of each square

// Piece movement properties
extern int g_pieceKnight[13];      // Is piece a knight?
extern int g_pieceKing[13];        // Is piece a king?
extern int g_pieceRookQueen[13];   // Does piece move like rook/queen?
extern int g_pieceBishopQueen[13]; // Does piece move like bishop/queen?
extern int g_pieceSlides[13];      // Is piece a sliding piece?

// Evaluation helper arrays
extern int Mirror64[64];  // Mirror table for position evaluation

// Bitboard masks for evaluation
extern U64 g_fileBBMask[8];         // Bitboard for each file
extern U64 g_rankBBMask[8];         // Bitboard for each rank
extern U64 g_blackPassedMask[64];   // Passed pawn masks for black
extern U64 g_whitePassedMask[64];   // Passed pawn masks for white
extern U64 g_isolatedMask[64];      // Isolated pawn masks

// Engine options
extern S_OPTIONS EngineOptions[1];

/* ===========================================================================
 * FUNCTION DECLARATIONS
 * ===========================================================================
 */

/* FUNCTIONS */

/* ---------------------------------------------------------------------------
 * INITIALIZATION (utils_init.c)
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
extern void Init_All();

/* ---------------------------------------------------------------------------
 * BITBOARD OPERATIONS (bitboards_utils.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Print a bitboard as an 8x8 grid for debugging
 * @param bb Bitboard to print
 */
extern void Bitboard_Print(U64 bb);

/**
 * @brief Remove and return the index of the least significant bit
 * @param bb Pointer to bitboard
 * @return Index of the LSB (0-63), or -1 if bitboard is empty
 */
extern int Bitboard_PopBit(U64 *bb);

/**
 * @brief Count the number of set bits in a bitboard
 * @param b Bitboard to count
 * @return Number of set bits
 */
extern int Bitboard_CountBits(U64 b);

/* ---------------------------------------------------------------------------
 * HASH KEYS (board_hashkeys.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Generate Zobrist hash key for current board position
 * @param board Board position
 * @return 64-bit hash key uniquely identifying the position
 */
extern U64 Board_GeneratePositionKey(const ChessBoard *board);

/* ---------------------------------------------------------------------------
 * BOARD OPERATIONS (board_representation.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Reset board to empty state
 * @param board Board to reset
 */
extern void Board_Reset(ChessBoard *board);

/**
 * @brief Parse FEN string and set up board position
 * @param fen FEN string to parse
 * @param board Board to set up
 * @return BOOL_TYPE_TRUE if successful, BOOL_TYPE_FALSE if FEN is invalid
 */
extern int Board_ParseFromFEN(char *fen, ChessBoard *board);

/**
 * @brief Print board position to console in ASCII art
 * @param board Board to print
 */
extern void Board_Print(const ChessBoard *board);

/**
 * @brief Update piece lists and material counts from board array
 * @param board Board to update
 * 
 * This function should be called after directly modifying the pieces array
 * to keep derived data structures in sync.
 */
extern void Board_UpdateListsMaterial(ChessBoard *board);

/**
 * @brief Validate board consistency for debugging
 * @param board Board to check
 * @return BOOL_TYPE_TRUE if board is valid, BOOL_TYPE_FALSE otherwise
 * 
 * Verifies:
 * - Piece lists match board array
 * - Bitboards match board array
 * - Material counts are correct
 * - Position hash is correct
 */
extern int Board_Check(const ChessBoard *board);

/**
 * @brief Mirror board position vertically (for evaluation testing)
 * @param board Board to mirror
 */
extern void Board_Mirror(ChessBoard *board);

/* ---------------------------------------------------------------------------
 * ATTACK DETECTION (attack_detection.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Check if a square is attacked by a given side
 * @param squareIndex Square to check (120-square index)
 * @param side Color of attacking side (COLOR_TYPE_WHITE or COLOR_TYPE_BLACK)
 * @param board Board position
 * @return BOOL_TYPE_TRUE if square is attacked, BOOL_TYPE_FALSE otherwise
 * 
 * Used for:
 * - Check detection
 * - Castle legality
 * - Move validation
 */
extern int Attack_IsSquareAttacked(const int squareIndex, const int side, const ChessBoard *board);

/* ---------------------------------------------------------------------------
 * INPUT/OUTPUT (moves_io.c)
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
 * @param squareIndex Square index (120-square)
 * @return String representation (e.g., "e4", "a1")
 * 
 * Note: Returns pointer to static buffer, not thread-safe
 */
extern char *PrSq(const int squareIndex);

/**
 * @brief Print all moves in a move list
 * @param list Move list to print
 */
extern void PrintMoveList(const MoveList *list);

/**
 * @brief Parse string to move (e.g., "e2e4" to encoded move)
 * @param ptrChar String to parse
 * @param board Current board position
 * @return Encoded move if valid, NOMOVE otherwise
 */
extern int Move_Parse(char *ptrChar, ChessBoard *board);


/* ---------------------------------------------------------------------------
 * VALIDATION (board_validate.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Check if square is on the board
 * @param squareIndex Square index (120-square)
 * @return BOOL_TYPE_TRUE if on board, BOOL_TYPE_FALSE if offboard or invalid
 */
extern int SqOnBoard(const int squareIndex);

/**
 * @brief Check if side value is valid
 * @param side Side to check
 * @return BOOL_TYPE_TRUE if COLOR_TYPE_WHITE or COLOR_TYPE_BLACK, BOOL_TYPE_FALSE otherwise
 */
extern int SideValid(const int side);

/**
 * @brief Check if file or rank is valid
 * @param fr File or rank value
 * @return BOOL_TYPE_TRUE if valid (0-7), BOOL_TYPE_FALSE otherwise
 */
extern int FileRankValid(const int fr);

/**
 * @brief Check if piece is valid or EMPTY
 * @param piece Piece value
 * @return BOOL_TYPE_TRUE if valid piece or EMPTY, BOOL_TYPE_FALSE otherwise
 */
extern int PieceValidEmpty(const int piece);

/**
 * @brief Check if piece is valid (not EMPTY)
 * @param piece Piece value
 * @return BOOL_TYPE_TRUE if valid piece, BOOL_TYPE_FALSE otherwise
 */
extern int PieceValid(const int piece);

/**
 * @brief Testing function for evaluation symmetry
 * @param board Board position
 */
extern void MirrorEvalTest(ChessBoard *board);

/**
 * @brief Check if square index is in 120-square format
 * @param squareIndex Square index
 * @return BOOL_TYPE_TRUE if valid 120-square index, BOOL_TYPE_FALSE otherwise
 */
extern int SqIs120(const int squareIndex);

/**
 * @brief Check if piece is valid, EMPTY, or OFFBOARD
 * @param piece Piece value
 * @return BOOL_TYPE_TRUE if valid, BOOL_TYPE_FALSE otherwise
 */
extern int PceValidEmptyOffbrd(const int piece);

/**
 * @brief Validate move list integrity
 * @param list Move list to check
 * @param board Board position
 * @return BOOL_TYPE_TRUE if valid, BOOL_TYPE_FALSE otherwise
 */
extern int MoveListOk(const MoveList *list,  const ChessBoard *board);

/**
 * @brief Debug function for analysis testing
 * @param board Board position
 * @param info Search information
 */
extern void DebugAnalysisTest(ChessBoard *board, SearchInfo *info);

/* ---------------------------------------------------------------------------
 * MOVE GENERATION (moves_generation.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Generate all pseudo-legal moves for current position
 * @param board Board position
 * @param list Output move list
 * 
 * Generates all moves including those that leave king in check.
 * Moves must be validated with Move_Make to ensure legality.
 */
extern void Move_GenerateAll(const ChessBoard *board, MoveList *list);

/**
 * @brief Generate all capture moves only
 * @param board Board position
 * @param list Output move list
 * 
 * Used in quiescence search to consider only tactical moves.
 */
extern void GenerateAllCaps(const ChessBoard *board, MoveList *list);

/**
 * @brief Check if a move exists in current position
 * @param board Board position
 * @param move Move to check
 * @return BOOL_TYPE_TRUE if move is legal, BOOL_TYPE_FALSE otherwise
 */
extern int MoveExists(ChessBoard *board, const int move);

/**
 * @brief Initialize MVV-LVA (Most Valuable Victim - Least Valuable Attacker) table
 * 
 * This table is used for move ordering to search captures in optimal order.
 */
extern void Init_MvvLva();

/* ---------------------------------------------------------------------------
 * MAKE/UNMAKE MOVES (moves_execution.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Make a move on the board
 * @param board Board position (modified)
 * @param move Move to make
 * @return BOOL_TYPE_TRUE if move is legal (king not in check after move), BOOL_TYPE_FALSE otherwise
 * 
 * Updates board state, hash key, and all derived information.
 * If move is illegal, board state is unchanged.
 */
extern int Move_Make(ChessBoard *board, int move);

/**
 * @brief Unmake the last move (rollback to previous position)
 * @param board Board position (modified)
 * 
 * Restores board to state before last Move_Make call.
 * Must be called in reverse order of Move_Make.
 */
extern void Move_Take(ChessBoard *board);

/**
 * @brief Make a null move (pass without moving)
 * @param board Board position (modified)
 * 
 * Used for null move pruning in search.
 * Only changes side to move and updates hash.
 */
extern void MakeNullMove(ChessBoard *board);

/**
 * @brief Unmake a null move
 * @param board Board position (modified)
 */
extern void TakeNullMove(ChessBoard *board);

/* ---------------------------------------------------------------------------
 * PERFT (search_perft.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Performance test - count all leaf nodes at given depth
 * @param depth Depth to search
 * @param board Board position
 * 
 * Used for debugging move generation and make/unmake functions.
 * Prints node counts for each root move.
 */
extern void Search_PerftTest(int depth, ChessBoard *board);

/* ---------------------------------------------------------------------------
 * SEARCH (search_algorithm.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Search for best move using iterative deepening
 * @param board Board position
 * @param info Search control information
 * 
 * Main search function using alpha-beta with various enhancements:
 * - Iterative deepening
 * - Principal variation search
 * - Null move pruning
 * - Transposition table
 * - Quiescence search
 */
extern void Search_Position(ChessBoard *board, SearchInfo *info);

/**
 * @brief Get best move for current position (wrapper for Search_Position)
 * @param board Board position
 * @param info Search control information
 * @return Best move found
 */
extern int Search_GetBestMove(ChessBoard *board, SearchInfo *info);

/* ---------------------------------------------------------------------------
 * MISC UTILITIES (utils_misc.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Get current time in milliseconds
 * @return Time in milliseconds
 */
extern int Misc_GetTimeMs();

/**
 * @brief Check for user input and handle GUI/console commands
 * @param info Search information (may set quit or stopped flags)
 */
extern void Misc_ReadInput(SearchInfo *info);

/* ---------------------------------------------------------------------------
 * TRANSPOSITION TABLE (hashtable_pv.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Initialize hash table with specified size
 * @param table Hash table to initialize
 * @param MB Size in megabytes
 * 
 * Allocates memory for hash table. Call before first use.
 */
extern void HashTable_Init(HashTable *table, const int MB);

/**
 * @brief Store position in hash table
 * @param board Board position
 * @param move Best move for this position
 * @param score Score for this position
 * @param flags Entry type (HFEXACT, HFALPHA, or HFBETA)
 * @param depth Depth at which position was searched
 */
extern void HashTable_StoreEntry(ChessBoard *board, const int move, int score, const int flags, const int depth);

/**
 * @brief Probe hash table for position
 * @param board Board position
 * @param move Output parameter for best move
 * @param score Output parameter for score
 * @param alpha Alpha bound
 * @param beta Beta bound
 * @param depth Current search depth
 * @return BOOL_TYPE_TRUE if usable entry found, BOOL_TYPE_FALSE otherwise
 */
extern int HashTable_ProbeEntry(ChessBoard *board, int *move, int *score, int alpha, int beta, int depth);

/**
 * @brief Get principal variation move from hash table
 * @param board Board position
 * @return Best move from hash table, or NOMOVE if not found
 */
extern int HashTable_ProbePvMove(const ChessBoard *board);

/**
 * @brief Extract principal variation line from hash table
 * @param depth Maximum depth
 * @param board Board position
 * @return Number of moves in PV line
 */
extern int HashTable_GetPvLine(const int depth, ChessBoard *board);

/**
 * @brief Clear all entries in hash table
 * @param table Hash table to clear
 */
extern void HashTable_Clear(HashTable *table);

/* ---------------------------------------------------------------------------
 * EVALUATION (evaluation_static.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Evaluate board position from current side's perspective
 * @param board Board position
 * @return Evaluation score in centipawns (positive = current side better)
 * 
 * Considers:
 * - Material balance
 * - Piece-square tables
 * - Pawn structure
 * - Piece mobility
 * - King safety
 */
extern int Evaluate_Position(const ChessBoard *board);

/**
 * @brief Test evaluation symmetry by mirroring position
 * @param board Board position
 */
extern void MirrorEvalTest(ChessBoard *board) ;

/* ---------------------------------------------------------------------------
 * UCI PROTOCOL (protocols_uci.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Main loop for UCI (Universal Chess Interface) protocol
 * @param board Board position
 * @param info Search information
 * 
 * Handles UCI commands from GUI:
 * - uci, isready, ucinewgame
 * - position, go, stop
 * - setoption
 */
extern void Uci_Loop(ChessBoard *board, SearchInfo *info);

/* ---------------------------------------------------------------------------
 * XBOARD PROTOCOL (protocols_xboard.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Main loop for XBoard/WinBoard protocol
 * @param board Board position
 * @param info Search information
 */
extern void XBoard_Loop(ChessBoard *board, SearchInfo *info);

/**
 * @brief Main loop for console mode (human play)
 * @param board Board position
 * @param info Search information
 */
extern void Console_Loop(ChessBoard *board, SearchInfo *info);

/**
 * @brief Check if game is over (checkmate, stalemate, draw)
 * @param board Board position
 * @return Result code (0 = game continues, non-zero = game over)
 */
extern int checkresult(ChessBoard *board);

/**
 * @brief Check if position is drawn by insufficient material
 * @param board Board position
 * @return BOOL_TYPE_TRUE if draw, BOOL_TYPE_FALSE otherwise
 * 
 * Recognizes:
 * - K vs K
 * - K+minor vs K
 * - K+B vs K+B (same color bishops)
 */
extern int DrawMaterial(const ChessBoard *board);

/**
 * @brief Check if position has occurred three times (draw by repetition)
 * @param board Board position
 * @return BOOL_TYPE_TRUE if threefold repetition, BOOL_TYPE_FALSE otherwise
 */
extern int ThreeFoldRep(const ChessBoard *board);

/* ---------------------------------------------------------------------------
 * POLYGLOT OPENING BOOK (openingbook_poly.c)
 * ---------------------------------------------------------------------------
 */

/**
 * @brief Get move from opening book for current position
 * @param board Board position
 * @return Move from book, or NOMOVE if position not in book
 */
extern int PolyBook_GetMove(ChessBoard *board);

/**
 * @brief Clean up opening book resources
 */
extern void PolyBook_Clean();

/**
 * @brief Initialize opening book
 * 
 * Loads opening book file if available.
 */
extern void PolyBook_Init() ;

#endif