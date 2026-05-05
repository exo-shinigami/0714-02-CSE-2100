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
#include <vector>

#include "../board/board_query_interface.h"

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
#define fILERANKTOSQUARE(f,r)  ( (21 + (f)) + ((r) * 10) )
#define SQUARE_120_TO_64(sq120)   (g_square120To64[(sq120)])
#define SQUARE_64_TO_120(sq64)    (g_square64To120[(sq64)])

#define BITBOARD_POP(b)           bitboardPopBit(b)
#define BITBOARD_COUNT(b)         bitboardCountBits(b)
#define BITBOARD_CLEAR_BIT(bb,sq) ((bb) &= g_bitClearMask[(sq)])
#define BITBOARD_SET_BIT(bb,sq)   ((bb) |= g_bitSetMask[(sq)])

#define PIECE_IS_BISHOP_QUEEN(p)  (g_pieceBishopQueen[(p)])
#define PIECE_IS_ROOK_QUEEN(p)    (g_pieceRookQueen[(p)])
#define PIECE_IS_KNIGHT(p)        (g_pieceKnight[(p)])
#define PIECE_IS_KING(p)          (g_pieceKing[(p)])
#define SQUARE_MIRROR_64(sq)      (mirror64[(sq)])

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
extern char pceChar[];
extern char sideChar[];
extern char rankChar[];
extern char fileChar[];
extern int  pieceBig[13];
extern int  pieceMaj[13];
extern int  pieceMin[13];
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
extern int  mirror64[64];
extern U64  g_fileBBMask[8];
extern U64  g_rankBBMask[8];
extern U64  g_blackPassedMask[64];
extern U64  g_whitePassedMask[64];
extern U64  g_isolatedMask[64];

/* ===========================================================================
 * BITBOARD UTILITIES (bitboards_utils.cpp)
 * ===========================================================================
 */
extern void bitboardPrint(U64 bb);
extern int  bitboardPopBit(U64 *bb);
extern int  bitboardCountBits(U64 b);

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
 * CLASS: move
 * Represents a single move with its move-ordering score.
 * All data is private; access via typed API.
 * ===========================================================================
 */
class move {
    int move_  = NOMOVE;
    int score_ = 0;
public:
    move() = default;
    move(int m, int s = 0) : move_(m), score_(s) {}

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
    move moves_[CHESS_MAX_POSITION_MOVES];
    int  count_ = 0;
public:
    MoveList() : count_(0) {}

    void clear()  { count_ = 0; }
    void print()  const;

    // Add a new move with a score
    void push(int moveValue, int score = 0) {
		if (count_ >= CHESS_MAX_POSITION_MOVES) {
			return;
		}
        moves_[count_] = move(moveValue, score);
        ++count_;
    }

    int         size()       const { return count_; }
    move&       at(int i)          { return moves_[i]; }
    const move& at(int i)    const { return moves_[i]; }
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
    std::vector<HashEntry> table_;
    int        numEntries = 0;
    int        newWrite   = 0;
    int        overWrite  = 0;
    int        hit        = 0;
    int        cut        = 0;
public:
    HashTable() = default;
    ~HashTable() = default;
    // Non-copyable (owns heap memory)
    HashTable(const HashTable&)            = delete;
    HashTable& operator=(const HashTable&) = delete;

    void init(int mB);
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
class ChessBoard : public IBoardQuery, public IBoardModifier, public IBoardSetup {
public:
    // History
    UndoMove  history[CHESS_MAX_GAME_MOVES];

private:
    // Board data
    int       pieces[CHESS_BOARD_SQUARE_NUM];

    U64       pawns[3];

    // Derived piece counts
    int       pieceCount[13];
    int       bigPce[2];
    int       majPce[2];
    int       minPce[2];
    int       material[2];

    int       pList[13][10];

    // Game state internals (encapsulated through getters/setters)
    int       kingSq[2];
    int       side;
    int       enPas;
    int       fiftyMove;
    int       ply;
    int       hisPly;
    int       castlePerm;
    U64       posKey;

    HashTable hashTable;

public:

    // Constructor — initialises to empty board
    ChessBoard();

    // Board management
    void  reset() override;
    int   parseFromFEN(const char *fen) override;
    void  print() const override;
    void  updateListsMaterial() override;
    int   check() const;
    void  mirror() override;
    U64   generatePositionKey() const;

    // move execution
    int   makeMove(int move) override;
    void  takeMove() override;
    void  makeNullMove() override;
    void  takeNullMove() override;
    int   hasAnyLegalMove() override;

    // Hash table operations (encapsulation layer)
    void initHashTable(int mB) { hashTable.init(mB); }
    void clearHashTable() { hashTable.clear(); }
    void clearHashStats() { hashTable.clearStats(); }
    int probeHashEntry(int *move, int *score, int alpha, int beta, int depth) {
        return hashTable.probeEntry(this, move, score, alpha, beta, depth);
    }
    void incrementHashCut() { hashTable.incrementCut(); }
    void storeHashEntry(int move, int score, int flags, int depth) {
        hashTable.storeEntry(this, move, score, flags, depth);
    }
    int getPvLine(int depth, SearchInfo *info) {
        return hashTable.getPvLine(depth, this, info);
    }

    // Attack query
    int   isSquareAttacked(int sq, int side) const override;

    // Read-only query helpers for incremental encapsulation
    int getPieceAt(int square) const override { return pieces[square]; }
    int& pieceAt(int square) { return pieces[square]; }
    int pieceAt(int square) const { return pieces[square]; }
    int getPieceCount(int piece) const { return pieceCount[piece]; }
    int& pieceCountAt(int piece) { return pieceCount[piece]; }
    int pieceCountAt(int piece) const { return pieceCount[piece]; }
    int getPieceSquare(int piece, int index) const override { return pList[piece][index]; }
    int& pieceListAt(int piece, int index) { return pList[piece][index]; }
    int pieceListAt(int piece, int index) const { return pList[piece][index]; }
    int getBigPieceCount(int color) const { return bigPce[color]; }
    int& bigPieceCountAt(int color) { return bigPce[color]; }
    int bigPieceCountAt(int color) const { return bigPce[color]; }
    U64 getPawns(int color) const override { return pawns[color]; }
    U64& pawnsAt(int color) { return pawns[color]; }
    U64 pawnsAt(int color) const { return pawns[color]; }
    int getMaterial(int color) const override { return material[color]; }
    int& materialAt(int color) { return material[color]; }
    int materialAt(int color) const { return material[color]; }
    int& majorPieceCountAt(int color) { return majPce[color]; }
    int majorPieceCountAt(int color) const { return majPce[color]; }
    int& minorPieceCountAt(int color) { return minPce[color]; }
    int minorPieceCountAt(int color) const { return minPce[color]; }
    int getSide() const override { return side; }
    void setSide(int value) { side = value; }
    void toggleSide() { side ^= 1; }
    int getEnPassantSquare() const override { return enPas; }
    void setEnPassantSquare(int value) { enPas = value; }
    int getCastlePermission() const override { return castlePerm; }
    void setCastlePermission(int value) { castlePerm = value; }
    void andCastlePermission(int mask) { castlePerm &= mask; }
    void orCastlePermission(int mask) { castlePerm |= mask; }
    int getFiftyMoveCounter() const override { return fiftyMove; }
    void setFiftyMoveCounter(int value) { fiftyMove = value; }
    void incrementFiftyMoveCounter() { ++fiftyMove; }
    int getPly() const { return ply; }
    void setPly(int plyValue) { ply = plyValue; }
    void incrementPly() { ++ply; }
    void decrementPly() { --ply; }
    int getKingSquare(int color) const override { return kingSq[color]; }
    void setKingSquare(int color, int square) { kingSq[color] = square; }
    U64 getPositionKey() const override { return posKey; }
    void setPositionKey(U64 value) { posKey = value; }
    void xorPositionKey(U64 value) { posKey ^= value; }
    int getHistoryPly() const override { return hisPly; }
    void setHistoryPly(int value) { hisPly = value; }
    void incrementHistoryPly() { ++hisPly; }
    void decrementHistoryPly() { --hisPly; }
    U64 getHistoryPositionKey(int index) const override { return history[index].posKey; }
    int isInCheck(int color) const override { return isSquareAttacked(kingSq[color], color ^ 1); }
    int isValid() const override { return check(); }
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
    int   gameMode     = MODE_TYPE_UCI;
    int   postThinking = BOOL_TYPE_FALSE;

    // Search-auxiliary tables (SRP: moved from ChessBoard — search data belongs here)
    int   pvArray[CHESS_MAX_SEARCH_DEPTH]              = {};
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

    void setPvMove(int index, int moveValue) {
        ASSERT(index >= 0 && index < CHESS_MAX_SEARCH_DEPTH);
        pvArray[index] = moveValue;
    }

    int getPvMove(int index) const {
        ASSERT(index >= 0 && index < CHESS_MAX_SEARCH_DEPTH);
        return pvArray[index];
    }

    void storeKillerMove(int ply, int moveValue) {
        ASSERT(ply >= 0 && ply < CHESS_MAX_SEARCH_DEPTH);
        searchKillers[1][ply] = searchKillers[0][ply];
        searchKillers[0][ply] = moveValue;
    }

    void addHistoryScore(int piece, int toSquare, int depthValue) {
        ASSERT(piece >= 0 && piece < 13);
        ASSERT(toSquare >= 0 && toSquare < CHESS_BOARD_SQUARE_NUM);
        searchHistory[piece][toSquare] += depthValue;
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
class IModel;

class IProtocol {
public:
    virtual ~IProtocol() = default;
    virtual void run(IModel& model, SearchInfo& info, const IEvaluator& eval) = 0;
};

class UciProtocol : public IProtocol {
public:
    void run(IModel& model, SearchInfo& info, const IEvaluator& eval) override;
};

class XBoardProtocol : public IProtocol {
public:
    void run(IModel& model, SearchInfo& info, const IEvaluator& eval) override;
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
 * CLASS: GameRulesService
 * Encapsulates terminal-state and draw-rule checks used by UIs/protocols.
 * ===========================================================================
 */
class GameRulesService {
public:
    static int repetitionCount(const IBoardQuery& board);
    static int isDrawByMaterial(const IBoardQuery& board);
    static int evaluateTerminalState(IBoardQuery& queryBoard, IBoardModifier& mutableBoard);
};

/* ===========================================================================
 * CLASS: PerftRunner
 * OOD wrapper for perft tree traversal and reporting.
 * ===========================================================================
 */
class PerftRunner {
    ChessBoard* board_ = nullptr;
    long leafNodes_ = 0;

    void runRecursive(int depth);
public:
    explicit PerftRunner(ChessBoard& board) : board_(&board) {}

    long run(int depth);
    void test(int depth);
};

/* ===========================================================================
 * CLASS: ValidationService
 * Encapsulates board and move-list validation/debug checks.
 * ===========================================================================
 */
class ValidationService {
public:
    static int isMoveListValid(const MoveList& list, const ChessBoard& board);
    static int isSquare120(int squareIndex);
    static int isPieceValidEmptyOrOffboard(int piece);
    static int isSquareOnBoard(int squareIndex);
    static int isSideValid(int side);
    static int isFileRankValid(int fileOrRank);
    static int isPieceValidEmpty(int piece);
    static int isPieceValid(int piece);
    static void runAnalysisTest(ChessBoard& board, SearchInfo& info, const IEvaluator& eval);
    static void runMirrorEvalTest(ChessBoard& board);
};

/* ===========================================================================
 * CLASS: EngineBootstrapService
 * Single initialization entry point for tables, masks, and hashing seeds.
 * ===========================================================================
 */
class EngineBootstrapService {
public:
    static void initEvalMasks();
    static void initFilesRanksBoard();
    static void initHashKeys();
    static void initBitMasks();
    static void initSquare120To64();
    static void initAll();
};

/* ===========================================================================
 * CLASS: RuntimeIOService
 * Encapsulates platform-dependent clock/input handling for search loops.
 * ===========================================================================
 */
class RuntimeIOService {
public:
    static int getTimeMs();
    static int isInputWaiting();
    static void readInput(SearchInfo& info);
};

/* ===========================================================================
 * VALIDATION FREE FUNCTIONS (board_validate.cpp)
 * ===========================================================================
 */
extern int  sqOnBoard(int sq);
extern int  sideValid(int side);
extern int  fileRankValid(int fr);
extern int  pieceValidEmpty(int piece);
extern int  pieceValid(int piece);
extern int  sqIs120(int sq);
extern int  pceValidEmptyOffbrd(int piece);
extern int  moveListOk(const MoveList *list, const ChessBoard *board);
extern void debugAnalysisTest(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern void mirrorEvalTest(ChessBoard *board);

/* ===========================================================================
 * MOVE GENERATION FREE FUNCTIONS (moves_generation.cpp)
 * ===========================================================================
 */
extern void moveGenerateAll(const ChessBoard *board, MoveList *list, const SearchInfo *info = nullptr);
extern void generateAllCaps(const ChessBoard *board, MoveList *list, const SearchInfo *info = nullptr);
extern int  moveExists(ChessBoard *board, int move);
extern void initMvvLva();

/* ===========================================================================
 * MOVE I/O FREE FUNCTIONS (moves_io.cpp)
 * ===========================================================================
 */
extern char *prMove(int move);
extern char *prSq(int sq);
extern void  printMoveList(const MoveList *list);
extern int   moveParse(const char *ptrChar, ChessBoard *board);

/* ===========================================================================
 * SEARCH FREE FUNCTIONS (search_algorithm.cpp)
 * ===========================================================================
 */
extern void searchPosition(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);
extern int  searchGetBestMove(ChessBoard *board, SearchInfo *info, const IEvaluator& eval);

/* ===========================================================================
 * pERFT (search_perft.cpp)
 * ===========================================================================
 */
extern void searchPerftTest(int depth, ChessBoard *board);

/* ===========================================================================
 * eVALUATION (evaluation_static.cpp)
 * ===========================================================================
 */
extern int evaluatePosition(const IBoardQuery *board);

/* ===========================================================================
 * MISC UTILITIES (utils_misc.cpp)
 * ===========================================================================
 */
extern int  miscGetTimeMs();
extern void miscReadInput(SearchInfo *info);

/* ===========================================================================
 * iNITIALIZATION (utils_init.cpp)
 * ===========================================================================
 */
extern void initAll();

/* ===========================================================================
 * OPENING BOOK (openingbook_poly.cpp)
 * ===========================================================================
 */
extern int  polyBookGetMove(ChessBoard *board);
extern void polyBookClean();
extern void polyBookInit();

/* ===========================================================================
 * GAME STATE HELPERS (core/board/game_rules.cpp)
 * ===========================================================================
 */
extern int checkresult(ChessBoard *board);
extern int drawMaterial(const IBoardQuery *board);
extern int threeFoldRep(const IBoardQuery *board);

#endif // DEFS_H
