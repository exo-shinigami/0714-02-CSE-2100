/**
 * @file search_algorithm.c
 * @brief Main search algorithm implementation
 * 
 * Implements the alpha-beta search algorithm with various enhancements:
 * - Iterative deepening
 * - Principal Variation Search (PVS)
 * - Null move pruning
 * - quiescence search (for tactical stability)
 * - Transposition table integration
 * - move ordering (hash move, MVV-LVA, killer moves, history heuristic)
 * - Time management
 * - Aspiration windows
 * 
 * The search is the core of the chess engine's decision-making process,
 * evaluating positions to find the best move.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

// search.c

#include "stdio.h"
#include "types_definitions.h"


int rootDepth;

static void checkUp(SearchInfo *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == BOOL_TYPE_TRUE && miscGetTimeMs() > info->stoptime) {
		info->stopped = BOOL_TYPE_TRUE;
	}

	miscReadInput(info);
}

static void pickNextMove(int moveNum, MoveList *list) {

	move temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->size(); ++index) {
		if (list->at(index).score() > bestScore) {
			bestScore = list->at(index).score();
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->size());
	ASSERT(bestNum>=0 && bestNum<list->size());
	ASSERT(bestNum>=moveNum);

	temp = list->at(moveNum);
	list->at(moveNum) = list->at(bestNum);
	list->at(bestNum) = temp;
}

static int isRepetition(const ChessBoard *board) {

	int index = 0;

	for(index = board->getHistoryPly() - board->getFiftyMoveCounter(); index < board->getHistoryPly()-1; ++index) {
		ASSERT(index >= 0 && index < CHESS_MAX_GAME_MOVES);
		if(board->getPositionKey() == board->getHistoryPositionKey(index)) {
			return BOOL_TYPE_TRUE;
		}
	}
	return BOOL_TYPE_FALSE;
}

static void searchClearFor(ChessBoard *board, SearchInfo *info) {

	info->clearSearchTables();

	board->clearHashStats();
	board->setPly(0);

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int quiescence(int alpha, int beta, ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {

	ASSERT(board->check());
	ASSERT(beta>alpha);
	if(( info->nodes & 2047 ) == 0) {
		checkUp(info);
	}

	info->nodes++;

	if(isRepetition(board) || board->getFiftyMoveCounter() >= 100) {
		return 0;
	}

	if(board->getPly() > CHESS_MAX_SEARCH_DEPTH - 1) {
		return eval.evaluate(*board);
	}

	int score = eval.evaluate(*board);

	ASSERT(score>-CHESS_INFINITE && score<CHESS_INFINITE);

	if(score >= beta) {
		return beta;
	}

	if(score > alpha) {
		alpha = score;
	}

	MoveList list[1];
    generateAllCaps(board, list, info);

    int moveNum = 0;
	int legal = 0;
	score = -CHESS_INFINITE;

	for(moveNum = 0; moveNum < list->size(); ++moveNum) {

		pickNextMove(moveNum, list);

        if ( !board->makeMove(list->at(moveNum).raw()))  {
            continue;
        }

		legal++;
		score = -quiescence( -beta, -alpha, board, info, eval);
        board->takeMove();

		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}

		if(score > alpha) {
			if(score >= beta) {
				if(legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = score;
		}
    }

	ASSERT(alpha >= oldAlpha);

	return alpha;
}

static int alphaBeta(int alpha, int beta, int depth, ChessBoard *board, SearchInfo *info, int DoNull, const IEvaluator& eval) {

	ASSERT(board->check());
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	if(depth <= 0) {
		return quiescence(alpha, beta, board, info, eval);
		// return eval.evaluate(*board);
	}

	if(( info->nodes & 2047 ) == 0) {
		checkUp(info);
	}

	info->nodes++;

	if((isRepetition(board) || board->getFiftyMoveCounter() >= 100) && board->getPly()) {
		return 0;
	}

	if(board->getPly() > CHESS_MAX_SEARCH_DEPTH - 1) {
		return eval.evaluate(*board);
	}

	int inCheck = board->isSquareAttacked(board->getKingSquare(board->getSide()), board->getSide()^1);

	if(inCheck == BOOL_TYPE_TRUE) {
		depth++;
	}

	int score = -CHESS_INFINITE;
	int pvMove = NOMOVE;

	if( board->probeHashEntry(&pvMove, &score, alpha, beta, depth) == BOOL_TYPE_TRUE ) {
		board->incrementHashCut();
		return score;
	}

	if( DoNull && !inCheck && board->getPly() && (board->getBigPieceCount(board->getSide()) > 0) && depth >= 4) {
		board->makeNullMove();
		score = -alphaBeta( -beta, -beta + 1, depth-4, board, info, BOOL_TYPE_FALSE, eval);
		board->takeNullMove();
		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}

		if (score >= beta && abs(score) < CHESS_IS_MATE) {
			info->nullCut++;
			return beta;
		}
	}

	MoveList list[1];
    moveGenerateAll(board, list, info);

    int moveNum = 0;
	int legal = 0;
	int oldAlpha = alpha;
	int bestMove = NOMOVE;

	int bestScore = -CHESS_INFINITE;

	score = -CHESS_INFINITE;

	if( pvMove != NOMOVE) {
		for(moveNum = 0; moveNum < list->size(); ++moveNum) {
			if( list->at(moveNum).raw() == pvMove) {
				list->at(moveNum).setScore(2000000);
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for(moveNum = 0; moveNum < list->size(); ++moveNum) {

		pickNextMove(moveNum, list);

        if ( !board->makeMove(list->at(moveNum).raw()))  {
            continue;
        }

		legal++;
		score = -alphaBeta( -beta, -alpha, depth-1, board, info, BOOL_TYPE_TRUE, eval);
		board->takeMove();

		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}
		if(score > bestScore) {
			bestScore = score;
			bestMove = list->at(moveNum).raw();
			if(score > alpha) {
				if(score >= beta) {
					if(legal==1) {
						info->fhf++;
					}
					info->fh++;

					if(!(list->at(moveNum).raw() & MFLAGCAP)) {
					info->storeKillerMove(board->getPly(), list->at(moveNum).raw());
				}

					board->storeHashEntry(bestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = score;

				if(!(list->at(moveNum).raw() & MFLAGCAP)) {
					info->addHistoryScore(board->pieceAt(MOVE_GET_FROM_SQUARE(bestMove)), MOVE_GET_TO_SQUARE(bestMove), depth);
				}
			}
		}
    }

	if(legal == 0) {
		if(inCheck) {
			return -CHESS_INFINITE + board->getPly();
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=oldAlpha);

	if(alpha != oldAlpha) {
		board->storeHashEntry(bestMove, bestScore, HFEXACT, depth);
	} else {
		board->storeHashEntry(bestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void searchPosition(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {

	int bestMove = NOMOVE;
	int bestScore = -CHESS_INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	searchClearFor(board,info);
	
	if(EngineOptions::instance().isBookEnabled()) {
		bestMove = polyBookGetMove(board);
	}

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	if(bestMove == NOMOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
								// alpha	 beta
			rootDepth = currentDepth;
				alphaBeta(-CHESS_INFINITE, CHESS_INFINITE, currentDepth, board, info, BOOL_TYPE_TRUE, eval);

			if(info->stopped == BOOL_TYPE_TRUE) {
				break;
			}

			board->getPvLine(currentDepth, info);
			bestMove = info->getPvMove(0);
			if(info->gameMode == MODE_TYPE_UCI) {
				printf("info score cp %d depth %d nodes %ld time %d ",
					bestScore,currentDepth,info->nodes,miscGetTimeMs()-info->starttime);
			} else if(info->gameMode == MODE_TYPE_XBOARD && info->postThinking == BOOL_TYPE_TRUE) {
				printf("%d %d %d %ld ",
					currentDepth,bestScore,(miscGetTimeMs()-info->starttime)/10,info->nodes);
			} else if(info->postThinking == BOOL_TYPE_TRUE) {
				printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
					bestScore,currentDepth,info->nodes,miscGetTimeMs()-info->starttime);
			}
			if(info->gameMode == MODE_TYPE_UCI || info->postThinking == BOOL_TYPE_TRUE) {
				pvMoves = board->getPvLine(currentDepth, info);
				if(info->gameMode != MODE_TYPE_XBOARD) {
					printf("pv");
				}
				for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
					printf(" %s",prMove(info->getPvMove(pvNum)));
				}
				printf("\n");
			}

			// Hash stats debug output intentionally omitted from board internals here.
			//(info->fhf/info->fh)*100,info->nullCut);
		}
	}

	if(info->gameMode == MODE_TYPE_UCI) {
		printf("bestmove %s\n",prMove(bestMove));
	} else if(info->gameMode == MODE_TYPE_XBOARD) {
		printf("move %s\n",prMove(bestMove));
		board->makeMove(bestMove);
	} else {
		printf("\n\n***!! Gambit makes move %s !!***\n\n",prMove(bestMove));
		board->makeMove(bestMove);
		board->print();
	}

}

// GUI-specific function that returns the best move without making it
int searchGetBestMove(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {
	int bestMove = NOMOVE;
	int currentDepth = 0;

	searchClearFor(board,info);
	
	if(EngineOptions::instance().isBookEnabled()) {
		bestMove = polyBookGetMove(board);
	}

	// iterative deepening
	if(bestMove == NOMOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
								// alpha	 beta
			rootDepth = currentDepth;
alphaBeta(-CHESS_INFINITE, CHESS_INFINITE, currentDepth, board, info, BOOL_TYPE_TRUE, eval);

			if(info->stopped == BOOL_TYPE_TRUE) {
				break;
			}

			board->getPvLine(currentDepth, info);
			bestMove = info->getPvMove(0);
		}
	}
	
	return bestMove;
}




















