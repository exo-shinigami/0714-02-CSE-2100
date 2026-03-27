/**
 * @file board_query_interface.h
 * @brief Read-only board query interface (ISP)
 * 
 * This interface follows Interface Segregation Principle by providing
 * only query methods. Clients that don't need to modify the board
 * can depend on this minimal interface.
 * 
 * @author Gambit Chess Team - OOD Refactored
 * @date March 2026
 * @version 2.1
 */

#ifndef BOARD_QUERY_INTERFACE_H
#define BOARD_QUERY_INTERFACE_H

/**
 * @class IBoardQuery
 * @brief Read-only interface for querying board state
 * 
 * SOLID Principles Applied:
 * - ISP: Minimal interface with only query methods
 * - SRP: Only provides read access to board state
 * 
 * Benefits:
 * - Evaluation functions can depend on this instead of full ChessBoard
 * - Prevents accidental board modification
 * - Makes code intent clearer (read-only access)
 */
class IBoardQuery {
public:
    virtual ~IBoardQuery() = default;
    
    // Position queries
    virtual int getPieceAt(int square) const = 0;
    virtual int getSide() const = 0;
    virtual int getEnPassantSquare() const = 0;
    virtual int getCastlePermission() const = 0;
    virtual int getFiftyMoveCounter() const = 0;
    
    // Piece queries
    virtual int getKingSquare(int side) const = 0;
    virtual int getPieceCount(int piece) const = 0;
    virtual int getPieceSquare(int piece, int index) const = 0;
    virtual int getMaterial(int side) const = 0;
    
    // Attack and validation
    virtual int isSquareAttacked(int square, int bySide) const = 0;
    virtual int isInCheck(int side) const = 0;
    
    // Bitboard access
    virtual unsigned long long getPawns(int side) const = 0;
    
    // Position key for hashing
    virtual unsigned long long getPositionKey() const = 0;

    // History queries
    virtual int getHistoryPly() const = 0;
    virtual unsigned long long getHistoryPositionKey(int index) const = 0;
    
    // Validation
    virtual int isValid() const = 0;
    
    // Display
    virtual void print() const = 0;
};

/**
 * @class IBoardModifier
 * @brief Interface for modifying board state
 * 
 * SOLID Principles Applied:
 * - ISP: Separate interface for modifications
 * - SRP: Only provides write access to board state
 * 
 * Benefits:
 * - Search functions need this in addition to IBoardQuery
 * - Clear separation between read and write operations
 */
class IBoardModifier {
public:
    virtual ~IBoardModifier() = default;
    
    // move execution
    virtual int makeMove(int move) = 0;
    virtual void takeMove() = 0;
    virtual void makeNullMove() = 0;
    virtual void takeNullMove() = 0;

    // Probe whether at least one legal move exists from current position
    virtual int hasAnyLegalMove() = 0;
    
    // Board setup
    virtual void reset() = 0;
    virtual int parseFromFEN(const char* fen) = 0;
    virtual void updateListsMaterial() = 0;
};

/**
 * @class IBoardSetup
 * @brief Interface for board initialization
 * 
 * SOLID Principles Applied:
 * - ISP: Separate interface for setup operations
 * - SRP: Only provides setup methods
 */
class IBoardSetup {
public:
    virtual ~IBoardSetup() = default;
    
    virtual void reset() = 0;
    virtual int parseFromFEN(const char* fen) = 0;
    virtual void mirror() = 0;
};

#endif // BOARD_QUERY_INTERFACE_H
