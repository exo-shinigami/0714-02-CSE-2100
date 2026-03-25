/**
 * @file types_definitions.h
 * @brief Core definitions, OOP classes, and declarations for Gambit Chess Engine
 *
 * All data structures are now C++ classes with member functions.
 * No C-style typedef struct remains.
 *
 * @author Gambit Chess Team
 * @date March 2026
 * @version 2.0 (OOP)
 */

#ifndef DEFS_H
#define DEFS_H

#include <cstdlib>
#include <cstdio>

// #define DEBUG

#define CHESS_MAX_HASH 1024

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

typedef unsigned long long U64;

#define NAME "Gambit 1.1"
#define CHESS_BOARD_SQUARE_NUM 120
#define CHESS_MAX_GAME_MOVES 2048
#define CHESS_MAX_POSITION_MOVES 256
#define CHESS_MAX_SEARCH_DEPTH 64
#define CHESS_START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define CHESS_INFINITE 30000
#define CHESS_IS_MATE (CHESS_INFINITE - CHESS_MAX_SEARCH_DEPTH)

/* ===========================================================================
 * ENUMERATIONS
 * ===========================================================================
 */

enum { EMPTY, PIECE_TYPE_WHITE_PAWN, PIECE_TYPE_WHITE_KNIGHT, PIECE_TYPE_WHITE_BISHOP,
       PIECE_TYPE_WHITE_ROOK, PIECE_TYPE_WHITE_QUEEN, PIECE_TYPE_WHITE_KING,
       PIECE_TYPE_BLACK_PAWN, PIECE_TYPE_BLACK_KNIGHT, PIECE_TYPE_BLACK_BISHOP,
       PIECE_TYPE_BLACK_ROOK, PIECE_TYPE_BLACK_QUEEN, PIECE_TYPE_BLACK_KING };

enum { FILE_TYPE_A, FILE_TYPE_B, FILE_TYPE_C, FILE_TYPE_D,
       FILE_TYPE_E, FILE_TYPE_F, FILE_TYPE_G, FILE_TYPE_H, FILE_TYPE_NONE };

enum { RANK_TYPE_1, RANK_TYPE_2, RANK_TYPE_3, RANK_TYPE_4,
       RANK_TYPE_5, RANK_TYPE_6, RANK_TYPE_7, RANK_TYPE_8, RANK_TYPE_NONE };

enum { COLOR_TYPE_WHITE, COLOR_TYPE_BLACK, COLOR_TYPE_BOTH };
enum { MODE_TYPE_UCI, MODE_TYPE_XBOARD, MODE_TYPE_CONSOLE };

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

enum { BOOL_TYPE_FALSE, BOOL_TYPE_TRUE };
enum { CASTLE_TYPE_WKCA = 1, CASTLE_TYPE_WQCA = 2, CASTLE_TYPE_BKCA = 4, CASTLE_TYPE_BQCA = 8 };
enum { HFNONE, HFALPHA, HFBETA, HFEXACT };

/* ===========================================================================
 * MOVE ENCODING MACROS
 * ===========================================================================
 * Bits 0-6:   From square
 * Bits 7-13:  To square
 * Bits 14-17: Captured piece
 * Bit  18:    En passant flag
 * Bit  19:    Pawn start flag
 * Bits 20-23: Promoted piece
 * Bit  24:    Castle flag
 */
#define MOVE_GET_FROM_SQUARE(m)   ((m) & 0x7F)
#define MOVE_GET_TO_SQUARE(m)     (((m) >> 7) & 0x7F)
#define MOVE_GET_CAPTURED(m)      (((m) >> 14) & 0xF)
#define MOVE_GET_PROMOTED(m)      (((m) >> 20) & 0xF)

#define MFLAGEP   0x40000
#define MFLAGPS   0x80000
#define MFLAGCA   0x1000000
#define MFLAGCAP  0x7C000
#define MFLAGPROM 0xF00000
#define NOMOVE    0

/* ===========================================================================
 * UTILITY MACROS
 * ===========================================================================
 */
#define FILE_RANK_TO_SQUARE(f,r)  ( (21 + (f)) + ((r) * 10) )
#define SQUARE_120_TO_64(sq120)   (g_square120To64[(sq120)])
#define SQUARE_64_TO_120(sq64)    (g_square64To120[(sq64)])

#define BITBOARD_POP(b)           Bitboard_PopBit(b)
#define BITBOARD_COUNT(b)         Bitboard_CountBits(b)
#define BITBOARD_CLEAR_BIT(bb,sq) ((bb) &= g_bitClearMask[(sq)])
#define BITBOARD_SET_BIT(bb,sq)   ((bb) |= g_bitSetMask[(sq)])

#define PIECE_IS_BISHOP_QUEEN(p)  (g_pieceBishopQueen[(p)])
#define PIECE_IS_ROOK_QUEEN(p)    (g_pieceRookQueen[(p)])
#define PIECE_IS_KNIGHT(p)        (g_pieceKnight[(p)])
#define PIECE_IS_KING(p)          (g_pieceKing[(p)])
#define SQUARE_MIRROR_64(sq)      (Mirror64[(sq)])

/* ===========================================================================
 * GLOBAL LOOKUP TABLES
 * ===========================================================================
 */
extern int  g_square120To64[CHESS_BOARD_SQUARE_NUM];
extern int  g_square64To120[64];
extern U64  g_bitSetMask[64];
extern U64  g_bitClearMask[64];
extern U64  g_pieceKeys[13][120];
extern U64  g_sideKey;
extern U64  g_castleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];
extern int  PieceBig[13];
extern int  PieceMaj[13];
extern int  PieceMin[13];
extern int  g_pieceVal[13];
extern int  g_pieceCol[13];
extern int  g_piecePawn[13];
extern int  g_filesBoard[CHESS_BOARD_SQUARE_NUM];
extern int  g_ranksBoard[CHESS_BOARD_SQUARE_NUM];
extern int  g_pieceKnight[13];
extern int  g_pieceKing[13];
extern int  g_pieceRookQueen[13];
extern int  g_pieceBishopQueen[13];
extern int  g_pieceSlides[13];
extern int  Mirror64[64];
extern U64  g_fileBBMask[8];
extern U64  g_rankBBMask[8];
extern U64  g_blackPassedMask[64];
extern U64  g_whitePassedMask[64];
extern U64  g_isolatedMask[64];

/* ===========================================================================
 * BITBOARD UTILITIES (bitboards_utils.cpp)
 * ===========================================================================
 */
extern void Bitboard_Print(U64 bb);
extern int  Bitboard_PopBit(U64 *bb);
extern int  Bitboard_CountBits(U64 b);

/* ===========================================================================
 * FORWARD DECLARATIONS
 * ===========================================================================
 */
class ChessBoard;
class SearchInfo;
class HashTable;

/* ===========================================================================
 * INTERFACE: IEvaluator  (Dependency Inversion Principle)
 * Search code depends on this abstraction, not on any concrete evaluator.
 * ===========================================================================
 */
class IEvaluator {
public:
    virtual ~IEvaluator() = default;
    virtual int evaluate(const ChessBoard& board) const = 0;
};

/* ===========================================================================
 * CLASS: Move
 * Represents a single move with its move-ordering score.
 * All data is private; access via typed API.
 * ===========================================================================
 */
class Move {
    int move_  = NOMOVE;
    int score_ = 0;
public:
    Move() = default;
    Move(int m, int s = 0) : move_(m), score_(s) {}

    // Raw integer encoding and score
    int  raw()         const { return move_; }
    int  score()       const { return score_; }
    void setScore(int s)     { score_ = s; }

    // Typed accessors
    int  fromSq()      const { return MOVE_GET_FROM_SQUARE(move_); }
    int  toSq()        const { return MOVE_GET_TO_SQUARE(move_); }
    int  captured()    const { return MOVE_GET_CAPTURED(move_); }
    int  promoted()    const { return MOVE_GET_PROMOTED(move_); }
    bool isEP()        const { return (move_ & MFLAGEP)   != 0; }
    bool isPawnStart() const { return (move_ & MFLAGPS)   != 0; }
    bool isCastle()    const { return (move_ & MFLAGCA)   != 0; }
    bool isCapture()   const { return (move_ & MFLAGCAP)  != 0; }
    bool isPromo()     const { return (move_ & MFLAGPROM) != 0; }
    bool isNull()      const { return move_ == NOMOVE; }
};

/* ===========================================================================
 * CLASS: MoveList
 * Ordered list of moves generated for a position.
 * Data is private; access via typed API.
 * ===========================================================================
 */
class MoveList {
    Move moves_[CHESS_MAX_POSITION_MOVES];
    int  count_ = 0;
public:
    MoveList() : count_(0) {}

    void clear()  { count_ = 0; }
    void print()  const;

    // Add a new move with a score
    void push(int move, int score = 0) {
        moves_[count_] = Move(move, score);
        ++count_;
    }

    int         size()       const { return count_; }
    Move&       at(int i)          { return moves_[i]; }
    const Move& at(int i)    const { return moves_[i]; }
};

/* ===========================================================================
 * CLASS: HashEntry
 * Single entry in the transposition table.
 * ===========================================================================
 */
class HashEntry {
public:
    U64 posKey = 0ULL;
    int move   = NOMOVE;
    int score  = 0;
    int depth  = 0;
    int flags  = HFNONE;
};

/* ===========================================================================
 * CLASS: HashTable
 * Transposition table — manages memory and all lookup operations.
 * RAII: destructor releases allocated memory automatically.
 * All data is private; manipulated only through member functions.
 * ===========================================================================
 */
class HashTable {
    HashEntry *pTable     = nullptr;
    int        numEntries = 0;
    int        newWrite   = 0;
    int        overWrite  = 0;
    int        hit        = 0;
    int        cut        = 0;
public:
    HashTable() = default;
    ~HashTable() { free(pTable); pTable = nullptr; }
    // Non-copyable (owns heap memory)
    HashTable(const HashTable&)            = delete;
    HashTable& operator=(const HashTable&) = delete;

    void init(int MB);
    void clear();
    void clearStats()   { overWrite = hit = cut = 0; }
    void incrementCut() { ++cut; }

    void storeEntry(ChessBoard *board, int move, int score, int flags, int depth);
    int  probeEntry(ChessBoard *board, int *move, int *score, int alpha, int beta, int depth);
    int  probePvMove(const ChessBoard *board) const;
    int  getPvLine(int depth, ChessBoard *board, SearchInfo *info);
};

/* ===========================================================================
 * CLASS: UndoMove
 * State snapshot required to reverse a move.
 * ===========================================================================
 */
class UndoMove {
public:
    int move        = NOMOVE;
    int castlePerm  = 0;
    int enPas       = NO_SQ;
    int fiftyMove   = 0;
    U64 posKey      = 0ULL;
};

/* ===========================================================================
 * CLASS: ChessBoard
 * Complete board state with all board-level operations as member functions.
 * ===========================================================================
 */
class ChessBoard {
public:
    // Board data
    int       pieces[CHESS_BOARD_SQUARE_NUM];
    U64       pawns[3];
    int       KingSq[2];

    // Game state
    int       side;
    int       enPas;
    int       fiftyMove;
    int       ply;
    int       hisPly;
    int       castlePerm;
    U64       posKey;

    // Derived piece counts
    int       pieceCount[13];
    int       bigPce[2];
    int       majPce[2];
    int       minPce[2];
    int       material[2];

    // History
    UndoMove  history[CHESS_MAX_GAME_MOVES];
    int       pList[13][10];
    HashTable hashTable;

    // Constructor — initialises to empty board
    ChessBoard();

    // Board management
    void  reset();
    int   parseFromFEN(const char *fen);
    void  print() const;
    void  updateListsMaterial();
    int   check() const;
    void  mirror();
    U64   generatePositionKey() const;

    // Move execution
    int   makeMove(int move);
    void  takeMove();
    void  makeNullMove();
    void  takeNullMove();

    // Attack query
    int   isSquareAttacked(int sq, int side) const;
};

/* ===========================================================================
 * CLASS: SearchInfo
 * Controls search parameters and accumulates statistics.
 * ===========================================================================
 */
class SearchInfo {
public:
    // Timing and flow control
    int   starttime     = 0;
    int   stoptime      = 0;
    int   depth         = 0;
    int   timeset       = BOOL_TYPE_FALSE;
    int   movestogo     = 0;
    long  nodes         = 0;
    int   quit          = BOOL_TYPE_FALSE;
    int   stopped       = BOOL_TYPE_FALSE;
    float fh            = 0.0f;
    float fhf           = 0.0f;
    int   nullCut       = 0;
    int   GAME_MODE     = MODE_TYPE_UCI;
    int   POST_THINKING = BOOL_TYPE_FALSE;

    // Search-auxiliary tables (SRP: moved from ChessBoard — search data belongs here)
    int   PvArray[CHESS_MAX_SEARCH_DEPTH]              = {};
    int   searchHistory[13][CHESS_BOARD_SQUARE_NUM]    = {};
    int   searchKillers[2][CHESS_MAX_SEARCH_DEPTH]     = {};

    SearchInfo() = default;

    void clearSearchTables() {
        for (int i = 0; i < 13; ++i)
            for (int j = 0; j < CHESS_BOARD_SQUARE_NUM; ++j)
                searchHistory[i][j] = 0;
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < CHESS_MAX_SEARCH_DEPTH; ++j)
                searchKillers[i][j] = 0;
    }
};

/* ===========================================================================
 * CLASS: StaticEvaluator  (implements IEvaluator)
 * Concrete evaluator using the static hand-crafted evaluation function.
 * ===========================================================================
 */
class StaticEvaluator : public IEvaluator {
public:
    int evaluate(const ChessBoard& board) const override;
};

/* ===========================================================================
 * INTERFACE: IProtocol  (Open/Closed + Dependency Inversion Principles)
 * Add new communication protocols without modifying existing code.
 * ===========================================================================
 */
class IProtocol {
public:
    virtual ~IProtocol() = default;
    virtual void run(ChessBoard& board, SearchInfo& info, const IEvaluator& eval) = 0;
};

class UciProtocol : public IProtocol {
public:
    void run(ChessBoard& board, SearchInfo& info, const IEvaluator& eval) override;
};

class XBoardProtocol : public IProtocol {
public:
    void run(ChessBoard& board, SearchInfo& info, const IEvaluator& eval) override;
};

/* ===========================================================================
 * CLASS: EngineOptions  — Singleton
 * Engine-level configuration options. Only one instance exists.
 * Obtain it via EngineOptions::instance().
 * ===========================================================================
 */
class EngineOptions {
    bool useBook_ = true;

    EngineOptions() = default;
    EngineOptions(const EngineOptions&)            = delete;
    EngineOptions& operator=(const EngineOptions&) = delete;
public:
    static EngineOptions& instance() {
        static EngineOptions inst;
        return inst;
    }

    bool isBookEnabled()       const { return useBook_; }
    void setBookEnabled(bool v)      { useBook_ = v; }
};

/* ===========================================================================
 * VALIDATION FREE FUNCTIONS (board_validate.cpp)
 * ===========================================================================
 */
extern int  SqOnBoard(int sq);
extern int  SideValid(int side);
extern int  FileRankValid(int fr);
extern int  PieceValidEmpty(int piece);
extern int  PieceValid(int piece);
extern int  SqIs120(int sq);
extern int  PceValidEmptyOffbrd(int piece);
extern int  MoveListOk(const MoveList *list, const ChessBoard *board);
extern void DebugAnalysisTest(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern void MirrorEvalTest(ChessBoard *board);

/* ===========================================================================
 * MOVE GENERATION FREE FUNCTIONS (moves_generation.cpp)
 * ===========================================================================
 */
extern void Move_GenerateAll(const ChessBoard *board, MoveList *list, const SearchInfo *info = nullptr);
extern void GenerateAllCaps(const ChessBoard *board, MoveList *list, const SearchInfo *info = nullptr);
extern int  MoveExists(ChessBoard *board, int move);
extern void Init_MvvLva();

/* ===========================================================================
 * MOVE I/O FREE FUNCTIONS (moves_io.cpp)
 * ===========================================================================
 */
extern char *PrMove(int move);
extern char *PrSq(int sq);
extern void  PrintMoveList(const MoveList *list);
extern int   Move_Parse(const char *ptrChar, ChessBoard *board);

/* ===========================================================================
 * SEARCH FREE FUNCTIONS (search_algorithm.cpp)
 * ===========================================================================
 */
extern void Search_Position(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern int  Search_GetBestMove(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);

/* ===========================================================================
 * PERFT (search_perft.cpp)
 * ===========================================================================
 */
extern void Search_PerftTest(int depth, ChessBoard *board);

/* ===========================================================================
 * EVALUATION (evaluation_static.cpp)
 * ===========================================================================
 */
extern int Evaluate_Position(const ChessBoard *board);

/* ===========================================================================
 * MISC UTILITIES (utils_misc.cpp)
 * ===========================================================================
 */
extern int  Misc_GetTimeMs();
extern void Misc_ReadInput(SearchInfo *info);

/* ===========================================================================
 * INITIALIZATION (utils_init.cpp)
 * ===========================================================================
 */
extern void Init_All();

/* ===========================================================================
 * OPENING BOOK (openingbook_poly.cpp)
 * ===========================================================================
 */
extern int  PolyBook_GetMove(ChessBoard *board);
extern void PolyBook_Clean();
extern void PolyBook_Init();

/* ===========================================================================
 * PROTOCOL LOOPS
 * ===========================================================================
 */
extern void Uci_Loop(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern void XBoard_Loop(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern void Console_Loop(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);

/* ===========================================================================
 * GAME STATE HELPERS (protocols_xboard.cpp)
 * ===========================================================================
 */
extern int checkresult(ChessBoard *board);
extern int DrawMaterial(const ChessBoard *board);
extern int ThreeFoldRep(const ChessBoard *board);

#endif // DEFS_H
