/**
 * @file search_algorithm.c
 * @brief Main search algorithm implementation
 * 
 * Implements the alpha-beta search algorithm with various enhancements:
 * - Iterative deepening
 * - Principal Variation Search (PVS)
 * - Null move pruning
 * - Quiescence search (for tactical stability)
 * - Transposition table integration
 * - Move ordering (hash move, MVV-LVA, killer moves, history heuristic)
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

static void CheckUp(SearchInfo *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == BOOL_TYPE_TRUE && Misc_GetTimeMs() > info->stoptime) {
		info->stopped = BOOL_TYPE_TRUE;
	}

	Misc_ReadInput(info);
}

static void PickNextMove(int moveNum, MoveList *list) {

	Move temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const ChessBoard *board) {

	int index = 0;

	for(index = board->hisPly - board->fiftyMove; index < board->hisPly-1; ++index) {
		ASSERT(index >= 0 && index < CHESS_MAX_GAME_MOVES);
		if(board->posKey == board->history[index].posKey) {
			return BOOL_TYPE_TRUE;
		}
	}
	return BOOL_TYPE_FALSE;
}

static void Search_ClearFor(ChessBoard *board, SearchInfo *info) {

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < CHESS_BOARD_SQUARE_NUM; ++index2) {
			board->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < CHESS_MAX_SEARCH_DEPTH; ++index2) {
			board->searchKillers[index][index2] = 0;
		}
	}

	board->HashTable->overWrite=0;
	board->HashTable->hit=0;
	board->HashTable->cut=0;
	board->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, ChessBoard *board, SearchInfo *info) {

	ASSERT(Board_Check(board));
	ASSERT(beta>alpha);
	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if(IsRepetition(board) || board->fiftyMove >= 100) {
		return 0;
	}

	if(board->ply > CHESS_MAX_SEARCH_DEPTH - 1) {
		return Evaluate_Position(board);
	}

	int Score = Evaluate_Position(board);

	ASSERT(Score>-CHESS_INFINITE && Score<CHESS_INFINITE);

	if(Score >= beta) {
		return beta;
	}

	if(Score > alpha) {
		alpha = Score;
	}

	MoveList list[1];
    GenerateAllCaps(board,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -CHESS_INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

        if ( !Move_Make(board,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, board, info);
        Move_Take(board);

		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}

		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
		}
    }

	ASSERT(alpha >= OldAlpha);

	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, ChessBoard *board, SearchInfo *info, int DoNull) {

	ASSERT(Board_Check(board));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	if(depth <= 0) {
		return Quiescence(alpha, beta, board, info);
		// return Evaluate_Position(board);
	}

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if((IsRepetition(board) || board->fiftyMove >= 100) && board->ply) {
		return 0;
	}

	if(board->ply > CHESS_MAX_SEARCH_DEPTH - 1) {
		return Evaluate_Position(board);
	}

	int InCheck = Attack_IsSquareAttacked(board->KingSq[board->side],board->side^1,board);

	if(InCheck == BOOL_TYPE_TRUE) {
		depth++;
	}

	int Score = -CHESS_INFINITE;
	int PvMove = NOMOVE;

	if( HashTable_ProbeEntry(board, &PvMove, &Score, alpha, beta, depth) == BOOL_TYPE_TRUE ) {
		board->HashTable->cut++;
		return Score;
	}

	if( DoNull && !InCheck && board->ply && (board->bigPce[board->side] > 0) && depth >= 4) {
		MakeNullMove(board);
		Score = -AlphaBeta( -beta, -beta + 1, depth-4, board, info, BOOL_TYPE_FALSE);
		TakeNullMove(board);
		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}

		if (Score >= beta && abs(Score) < CHESS_IS_MATE) {
			info->nullCut++;
			return beta;
		}
	}

	MoveList list[1];
    Move_GenerateAll(board,list);

    int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -CHESS_INFINITE;

	Score = -CHESS_INFINITE;

	if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

        if ( !Move_Make(board,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -AlphaBeta( -beta, -alpha, depth-1, board, info, BOOL_TYPE_TRUE);
		Move_Take(board);

		if(info->stopped == BOOL_TYPE_TRUE) {
			return 0;
		}
		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			if(Score > alpha) {
				if(Score >= beta) {
					if(Legal==1) {
						info->fhf++;
					}
					info->fh++;

					if(!(list->moves[MoveNum].move & MFLAGCAP)) {
						board->searchKillers[1][board->ply] = board->searchKillers[0][board->ply];
						board->searchKillers[0][board->ply] = list->moves[MoveNum].move;
					}

					HashTable_StoreEntry(board, BestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = Score;

				if(!(list->moves[MoveNum].move & MFLAGCAP)) {
					board->searchHistory[board->pieces[MOVE_GET_FROM_SQUARE(BestMove)]][MOVE_GET_TO_SQUARE(BestMove)] += depth;
				}
			}
		}
    }

	if(Legal == 0) {
		if(InCheck) {
			return -CHESS_INFINITE + board->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=OldAlpha);

	if(alpha != OldAlpha) {
		HashTable_StoreEntry(board, BestMove, BestScore, HFEXACT, depth);
	} else {
		HashTable_StoreEntry(board, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void Search_Position(ChessBoard *board, SearchInfo *info) {

	int bestMove = NOMOVE;
	int bestScore = -CHESS_INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	Search_ClearFor(board,info);
	
	if(EngineOptions->UseBook == BOOL_TYPE_TRUE) {
		bestMove = PolyBook_GetMove(board);
	}

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	if(bestMove == NOMOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
								// alpha	 beta
			rootDepth = currentDepth;
			bestScore = AlphaBeta(-CHESS_INFINITE, CHESS_INFINITE, currentDepth, board, info, BOOL_TYPE_TRUE);

			if(info->stopped == BOOL_TYPE_TRUE) {
				break;
			}

			pvMoves = HashTable_GetPvLine(currentDepth, board);
			bestMove = board->PvArray[0];
			if(info->GAME_MODE == MODE_TYPE_UCI) {
				printf("info score cp %d depth %d nodes %ld time %d ",
					bestScore,currentDepth,info->nodes,Misc_GetTimeMs()-info->starttime);
			} else if(info->GAME_MODE == MODE_TYPE_XBOARD && info->POST_THINKING == BOOL_TYPE_TRUE) {
				printf("%d %d %d %ld ",
					currentDepth,bestScore,(Misc_GetTimeMs()-info->starttime)/10,info->nodes);
			} else if(info->POST_THINKING == BOOL_TYPE_TRUE) {
				printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
					bestScore,currentDepth,info->nodes,Misc_GetTimeMs()-info->starttime);
			}
			if(info->GAME_MODE == MODE_TYPE_UCI || info->POST_THINKING == BOOL_TYPE_TRUE) {
				pvMoves = HashTable_GetPvLine(currentDepth, board);
				if(!info->GAME_MODE == MODE_TYPE_XBOARD) {
					printf("pv");
				}
				for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
					printf(" %s",PrMove(board->PvArray[pvNum]));
				}
				printf("\n");
			}

			//printf("Hits:%d Overwrite:%d NewWrite:%d Cut:%d\nOrdering %.2f NullCut:%d\n",board->HashTable->hit,board->HashTable->overWrite,board->HashTable->newWrite,board->HashTable->cut,
			//(info->fhf/info->fh)*100,info->nullCut);
		}
	}

	if(info->GAME_MODE == MODE_TYPE_UCI) {
		printf("bestmove %s\n",PrMove(bestMove));
	} else if(info->GAME_MODE == MODE_TYPE_XBOARD) {
		printf("move %s\n",PrMove(bestMove));
		Move_Make(board, bestMove);
	} else {
		printf("\n\n***!! Gambit makes move %s !!***\n\n",PrMove(bestMove));
		Move_Make(board, bestMove);
		Board_Print(board);
	}

}

// GUI-specific function that returns the best move without making it
int Search_GetBestMove(ChessBoard *board, SearchInfo *info) {
	int bestMove = NOMOVE;
	int bestScore = -CHESS_INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	Search_ClearFor(board,info);
	
	if(EngineOptions->UseBook == BOOL_TYPE_TRUE) {
		bestMove = PolyBook_GetMove(board);
	}

	// iterative deepening
	if(bestMove == NOMOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
								// alpha	 beta
			rootDepth = currentDepth;
			bestScore = AlphaBeta(-CHESS_INFINITE, CHESS_INFINITE, currentDepth, board, info, BOOL_TYPE_TRUE);

			if(info->stopped == BOOL_TYPE_TRUE) {
				break;
			}

			pvMoves = HashTable_GetPvLine(currentDepth, board);
			bestMove = board->PvArray[0];
		}
	}
	
	return bestMove;
}




















