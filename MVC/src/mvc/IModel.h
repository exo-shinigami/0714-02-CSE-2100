// IModel.h - MVC Model interface
#ifndef SOLID_MVC_IMODEL_H
#define SOLID_MVC_IMODEL_H

#include <string>

class ChessBoard;

struct Move {
    std::string san; // simple algebraic notation or UCI-like string
    int from;
    int to;
};

class IModel {
public:
    virtual ~IModel() {}

    // Initialize model resources. Returns true on success.
    virtual bool initialize() = 0;

    // Start a new game (reset board state)
    virtual void newGame() = 0;

    // Return pointer to internal board representation (may be nullptr)
    virtual ChessBoard* getBoard() = 0;

    // Apply a move. Returns true on success.
    virtual bool applyMove(const Move& m) = 0;

    // Apply a fully-validated raw move (internal move integer).
    virtual bool applyMoveRaw(int move) = 0;

    // Undo last move
    virtual void undoMove() = 0;

    // Side to move (0 = white, 1 = black)
    virtual int getSideToMove() const = 0;

    // UCI "position ..." (full line as read from stdin, including trailing newline allowed)
    virtual bool applyUciPositionLine(const std::string& line) = 0;

    // XBoard/console: reset hash table and reload start position (e.g. "new")
    virtual void protocolXBoardNewGame() = 0;

    // Load arbitrary FEN (e.g. XBoard setboard remainder); resets adapter move history stacks
    virtual bool setBoardFromFenPayload(const std::string& fenField) = 0;

    // Apply a single UCI token without re-fetching pointer (used by console / XBoard usermove)
    virtual bool applyUciCoordToken(const char* coordToken) = 0;

    virtual void configureHashMegabytes(int megabytes) = 0;

    virtual void clearTranspositionTable() = 0;

    virtual void resetSearchRootPly() = 0;
};

#endif // SOLID_MVC_IMODEL_H
