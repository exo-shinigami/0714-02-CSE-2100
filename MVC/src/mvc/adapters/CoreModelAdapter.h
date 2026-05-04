// CoreModelAdapter.h - lightweight adapter from IModel to existing core
#ifndef SOLID_MVC_COREMODELADAPTER_H
#define SOLID_MVC_COREMODELADAPTER_H

#include "../IModel.h"
#include "types_definitions.h"
#include <vector>
#include <string>

class CoreModelAdapter : public IModel {
public:
    CoreModelAdapter();
    virtual ~CoreModelAdapter();

    bool initialize() override;
    void newGame() override;
    ChessBoard* getBoard() override;
    bool applyMove(const Move& m) override;
    bool applyMoveRaw(int move) override;
    void undoMove() override;
    int getSideToMove() const override;

    bool applyUciPositionLine(const std::string& line) override;
    void protocolXBoardNewGame() override;
    bool setBoardFromFenPayload(const std::string& fenField) override;
    bool applyUciCoordToken(const char* coordToken) override;
    void configureHashMegabytes(int megabytes) override;
    void clearTranspositionTable() override;
    void resetSearchRootPly() override;

    // Additional convenience methods on adapter
    bool redoMove();
    std::vector<std::string> getHistoryUci() const;

private:
    bool applyInternalMove(int move);

    ChessBoard board_;
    bool initialized_;
    // History stacks store internal move integers (as used by core)
    std::vector<int> historyMoves_;
    std::vector<int> redoMoves_;
};

#endif // SOLID_MVC_COREMODELADAPTER_H
