/**
 * @file promotion_service.h
 * @brief Promotion domain rules and dialog-choice mapping
 */

#ifndef PROMOTION_SERVICE_H
#define PROMOTION_SERVICE_H

#include "types_definitions.h"

class PromotionService {
public:
    // Board-rule check for whether a move is a pawn promotion move.
    static bool isPromotionMove(const ChessBoard& board, int fromSq, int toSq);

    // Map promotion dialog click coordinates to promotion piece choice.
    // Returns '\0' when no promotion piece was selected.
    static char dialogChoiceFromClick(int mouseX, int mouseY, int windowWidth, int windowHeight);
};

#endif // PROMOTION_SERVICE_H
