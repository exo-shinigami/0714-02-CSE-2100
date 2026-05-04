/**
 * @file engine_move_policy.h
 * @brief Engine move policy abstractions for GUI play modes
 */

#ifndef ENGINE_MOVE_POLICY_H
#define ENGINE_MOVE_POLICY_H

#include "types_definitions.h"


/**
 * @brief Strategy interface for choosing an engine move.
 */
class IEngineMovePolicy {
public:
    virtual ~IEngineMovePolicy() = default;
    virtual int selectMove(ChessBoard& board, const IEvaluator& evaluator) const = 0;
};

/**
 * @brief Selects the move with the best immediate static evaluation.
 */
class GreedyEvalMovePolicy : public IEngineMovePolicy {
public:
    int selectMove(ChessBoard& board, const IEvaluator& evaluator) const override;
};

/**
 * @brief Returns the default engine move policy used by SDL GUI mode.
 */
const IEngineMovePolicy& defaultEngineMovePolicy();

#endif // ENGINE_MOVE_POLICY_H
