// CoreModelAdapter.cpp — IModel façade over ChessBoard and core move/history helpers.
#include "CoreModelAdapter.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

CoreModelAdapter::CoreModelAdapter()
    : board_(),
      initialized_(false) {}

CoreModelAdapter::~CoreModelAdapter() {}

bool CoreModelAdapter::initialize() {
    // Global initAll() must be called once from main before constructing positions.
    board_.initHashTable(64);
    if (board_.parseFromFEN(CHESS_START_FEN) != 0) {
        std::printf("CoreModelAdapter: failed to parse start FEN\n");
        return false;
    }

    initialized_ = true;
    historyMoves_.clear();
    redoMoves_.clear();
    return true;
}

void CoreModelAdapter::newGame() {
    if (!initialized_) return;

    historyMoves_.clear();
    redoMoves_.clear();
    board_.reset();
    board_.parseFromFEN(CHESS_START_FEN);
}

ChessBoard* CoreModelAdapter::getBoard() {
    return initialized_ ? &board_ : nullptr;
}

bool CoreModelAdapter::applyInternalMove(int move) {
    if (!initialized_ || move == NOMOVE) {
        return false;
    }
    if (!board_.makeMove(move)) {
        return false;
    }
    historyMoves_.push_back(move);
    redoMoves_.clear();
    return true;
}

bool CoreModelAdapter::applyMove(const Move& m) {
    if (!initialized_) return false;

    int move = NOMOVE;
    if (!m.san.empty()) {
        move = moveParse(m.san.c_str(), &board_);
    } else if (sqOnBoard(m.from) && sqOnBoard(m.to)) {
        const int fromFile = g_filesBoard[m.from];
        const int fromRank = g_ranksBoard[m.from];
        const int toFile = g_filesBoard[m.to];
        const int toRank = g_ranksBoard[m.to];

        if (fromFile != OFFBOARD && fromRank != OFFBOARD &&
            toFile != OFFBOARD && toRank != OFFBOARD) {
            char uci[6] = {};
            uci[0] = static_cast<char>('a' + fromFile);
            uci[1] = static_cast<char>('1' + fromRank);
            uci[2] = static_cast<char>('a' + toFile);
            uci[3] = static_cast<char>('1' + toRank);
            uci[4] = '\0';
            move = moveParse(uci, &board_);
        }
    }

    return applyInternalMove(move);
}

bool CoreModelAdapter::applyMoveRaw(int move) {
    return applyInternalMove(move);
}

void CoreModelAdapter::undoMove() {
    if (!initialized_) return;
    if (board_.getHistoryPly() <= 0) return;
    if (!historyMoves_.empty()) {
        int last = historyMoves_.back();
        historyMoves_.pop_back();
        redoMoves_.push_back(last);
    }
    board_.takeMove();
}

bool CoreModelAdapter::redoMove() {
    if (!initialized_) return false;
    if (redoMoves_.empty()) return false;
    int m = redoMoves_.back();
    redoMoves_.pop_back();
    return applyInternalMove(m);
}

std::vector<std::string> CoreModelAdapter::getHistoryUci() const {
    std::vector<std::string> out;
    for (int mv : historyMoves_) {
        const char* s = prMove(mv);
        if (s) out.emplace_back(s);
        else out.emplace_back("");
    }
    return out;
}

int CoreModelAdapter::getSideToMove() const {
    return initialized_ ? board_.getSide() : COLOR_TYPE_WHITE;
}

bool CoreModelAdapter::applyUciPositionLine(const std::string& lineStr) {
    if (!initialized_) return false;

    historyMoves_.clear();
    redoMoves_.clear();

    const char *lineIn = lineStr.c_str();
    if (std::strlen(lineIn) < 9) {
        return false;
    }
    lineIn += 9;

    const char *ptrChar = lineIn;

    if (strncmp(lineIn, "startpos", 8) == 0) {
        board_.parseFromFEN(CHESS_START_FEN);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if (ptrChar == NULL) {
            board_.parseFromFEN(CHESS_START_FEN);
        } else {
            ptrChar += 4;
            board_.parseFromFEN(ptrChar);
        }
    }

    ptrChar = strstr(lineIn, "moves");

    if (ptrChar != NULL) {
        ptrChar += 6;
        while (*ptrChar) {
            int move = moveParse(ptrChar, &board_);
            if (move == NOMOVE) {
                break;
            }
            if (!applyInternalMove(move)) {
                break;
            }
            board_.setPly(0);
            while (*ptrChar && *ptrChar != ' ') ptrChar++;
            ptrChar++;
        }
    }
    board_.print();
    return true;
}

void CoreModelAdapter::protocolXBoardNewGame() {
    if (!initialized_) return;

    board_.clearHashTable();
    historyMoves_.clear();
    redoMoves_.clear();
    board_.reset();
    board_.parseFromFEN(CHESS_START_FEN);
}

bool CoreModelAdapter::setBoardFromFenPayload(const std::string& fenField) {
    if (!initialized_) return false;

    historyMoves_.clear();
    redoMoves_.clear();
    return board_.parseFromFEN(fenField.c_str()) == 0;
}

bool CoreModelAdapter::applyUciCoordToken(const char* coordToken) {
    if (!initialized_ || coordToken == nullptr) return false;

    Move m{};
    m.san = coordToken;
    return applyMove(m);
}

void CoreModelAdapter::configureHashMegabytes(int megabytes) {
    if (!initialized_) return;

    int mB = megabytes;
    if (mB < 4) mB = 4;
    if (mB > CHESS_MAX_HASH) mB = CHESS_MAX_HASH;
    board_.initHashTable(mB);
}

void CoreModelAdapter::clearTranspositionTable() {
    if (!initialized_) return;
    board_.clearHashTable();
}

void CoreModelAdapter::resetSearchRootPly() {
    if (!initialized_) return;
    board_.setPly(0);
}
