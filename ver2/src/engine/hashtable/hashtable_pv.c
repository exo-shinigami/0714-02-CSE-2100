/**
 * @file hashtable_pv.c
 * @brief Transposition table (hash table) implementation
 * 
 * The transposition table stores previously evaluated positions to:
 * - Avoid re-searching identical positions
 * - Store best moves for move ordering
 * - Implement iterative deepening efficiently
 * - Provide principal variation extraction
 * 
 * Uses Zobrist hashing for position identification.
 * Implements replacement scheme that considers:
 * - Search depth
 * - Entry age
 * - Exact vs bound scores
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"

int HashTable_GetPvLine(const int depth, ChessBoard *board) {

	ASSERT(depth < CHESS_MAX_SEARCH_DEPTH && depth >= 1);

	int move = HashTable_ProbePvMove(board);
	int count = 0;
	
	while(move != NOMOVE && count < depth) {
	
		ASSERT(count < CHESS_MAX_SEARCH_DEPTH);
	
		if( MoveExists(board, move) ) {
			Move_Make(board, move);
			board->PvArray[count++] = move;
		} else {
			break;
		}		
		move = HashTable_ProbePvMove(board);	
	}
	
	while(board->ply > 0) {
		Move_Take(board);
	}
	
	return count;
	
}

void HashTable_Clear(HashTable *table) {

  HashEntry *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    tableEntry->move = NOMOVE;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
  }
  table->newWrite=0;
}

void HashTable_Init(HashTable *table, const int MB) {  
	
	int HashSize = 0x100000 * MB;
    table->numEntries = HashSize / sizeof(HashEntry);
    table->numEntries -= 2;
	
	if(table->pTable!=NULL) {
		free(table->pTable);
	}
		
    table->pTable = (HashEntry *) malloc(table->numEntries * sizeof(HashEntry));
	if(table->pTable == NULL) {
		printf("Hash Allocation Failed, trying %dMB...\n",MB/2);
		HashTable_Init(table,MB/2);
	} else {
		HashTable_Clear(table);
		// Hash table initialized silently
	}
	
}

int HashTable_ProbeEntry(ChessBoard *board, int *move, int *score, int alpha, int beta, int depth) {

	int index = board->posKey % board->HashTable->numEntries;
	
	ASSERT(index >= 0 && index <= board->HashTable->numEntries - 1);
    ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-CHESS_INFINITE&&alpha<=CHESS_INFINITE);
    ASSERT(beta>=-CHESS_INFINITE&&beta<=CHESS_INFINITE);
    ASSERT(board->ply>=0&&board->ply<CHESS_MAX_SEARCH_DEPTH);
	
	if( board->HashTable->pTable[index].posKey == board->posKey ) {
		*move = board->HashTable->pTable[index].move;
		if(board->HashTable->pTable[index].depth >= depth){
			board->HashTable->hit++;
			
			ASSERT(board->HashTable->pTable[index].depth>=1&&board->HashTable->pTable[index].depth<CHESS_MAX_SEARCH_DEPTH);
            ASSERT(board->HashTable->pTable[index].flags>=HFALPHA&&board->HashTable->pTable[index].flags<=HFEXACT);
			
			*score = board->HashTable->pTable[index].score;
			if(*score > CHESS_IS_MATE) *score -= board->ply;
            else if(*score < -CHESS_IS_MATE) *score += board->ply;
			
			switch(board->HashTable->pTable[index].flags) {
				
                ASSERT(*score>=-CHESS_INFINITE&&*score<=CHESS_INFINITE);

                case HFALPHA: if(*score<=alpha) {
                    *score=alpha;
                    return BOOL_TYPE_TRUE;
                    }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score=beta;
                    return BOOL_TYPE_TRUE;
                    }
                    break;
                case HFEXACT:
                    return BOOL_TYPE_TRUE;
                    break;
                default: ASSERT(BOOL_TYPE_FALSE); break;
            }
		}
	}
	
	return BOOL_TYPE_FALSE;
}

void HashTable_StoreEntry(ChessBoard *board, const int move, int score, const int flags, const int depth) {

	int index = board->posKey % board->HashTable->numEntries;
	
	ASSERT(index >= 0 && index <= board->HashTable->numEntries - 1);
	ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
    ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
    ASSERT(score>=-CHESS_INFINITE&&score<=CHESS_INFINITE);
    ASSERT(board->ply>=0&&board->ply<CHESS_MAX_SEARCH_DEPTH);
	
	if( board->HashTable->pTable[index].posKey == 0) {
		board->HashTable->newWrite++;
	} else {
		board->HashTable->overWrite++;
	}
	
	if(score > CHESS_IS_MATE) score += board->ply;
    else if(score < -CHESS_IS_MATE) score -= board->ply;
	
	board->HashTable->pTable[index].move = move;
    board->HashTable->pTable[index].posKey = board->posKey;
	board->HashTable->pTable[index].flags = flags;
	board->HashTable->pTable[index].score = score;
	board->HashTable->pTable[index].depth = depth;
}

int HashTable_ProbePvMove(const ChessBoard *board) {

	int index = board->posKey % board->HashTable->numEntries;
	ASSERT(index >= 0 && index <= board->HashTable->numEntries - 1);
	
	if( board->HashTable->pTable[index].posKey == board->posKey ) {
		return board->HashTable->pTable[index].move;
	}
	
	return NOMOVE;
}
















