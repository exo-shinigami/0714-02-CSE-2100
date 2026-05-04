/**
 * @file promotion_service.cpp
 * @brief PromotionService implementation
 */

#include "promotion_service.h"

bool PromotionService::isPromotionMove(const ChessBoard& board, int fromSq, int toSq) {
    const int piece = board.pieceAt(fromSq);

    if (!g_piecePawn[piece]) {
        return false;
    }

    const int toRank = g_ranksBoard[toSq];
    if (piece == PIECE_TYPE_WHITE_PAWN && toRank == RANK_TYPE_8) {
        return true;
    }
    if (piece == PIECE_TYPE_BLACK_PAWN && toRank == RANK_TYPE_1) {
        return true;
    }

    return false;
}

char PromotionService::dialogChoiceFromClick(int mouseX, int mouseY, int windowWidth, int windowHeight) {
    const int dialogW = 400;
    const int dialogH = 250;
    const int dialogX = (windowWidth - dialogW) / 2;
    const int dialogY = (windowHeight - dialogH) / 2;

    const int pieceSize = 80;
    const int spacing = 20;
    const int startX = dialogX + (dialogW - (4 * pieceSize + 3 * spacing)) / 2;
    const int startY = dialogY + 70;

    for (int i = 0; i < 4; ++i) {
        const int x = startX + i * (pieceSize + spacing);
        if (mouseX >= x && mouseX < x + pieceSize && mouseY >= startY && mouseY < startY + pieceSize) {
            static const char choices[4] = {'q', 'r', 'b', 'n'};
            return choices[i];
        }
    }

    return '\0';
}
