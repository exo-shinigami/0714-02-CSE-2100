/**
 * @file hashtable_pv.cpp
 * @brief Transposition table (hash table) implementation — HashTable member functions
 *
 * @author Gambit Chess Team
 * @date March 2026
 */

#include "stdio.h"
#include "types_definitions.h"

int HashTable::getPvLine(int depth, ChessBoard *board, SearchInfo *info) {

	ASSERT(depth < CHESS_MAX_SEARCH_DEPTH && depth >= 1);

	int move = probePvMove(board);
	int count = 0;

	while(move != NOMOVE && count < depth) {

		ASSERT(count < CHESS_MAX_SEARCH_DEPTH);

		if( MoveExists(board, move) ) {
			board->makeMove(move);
			info->PvArray[count++] = move;
		} else {
			break;
		}
		move = probePvMove(board);
	}

	while(board->ply > 0) {
		board->takeMove();
	}

	return count;
}

void HashTable::clear() {

	HashEntry *tableEntry;

	for (tableEntry = pTable; tableEntry < pTable + numEntries; tableEntry++) {
		tableEntry->posKey = 0ULL;
		tableEntry->move = NOMOVE;
		tableEntry->depth = 0;
		tableEntry->score = 0;
		tableEntry->flags = 0;
	}
	newWrite = 0;
}

void HashTable::init(int MB) {

	int HashSize = 0x100000 * MB;
	numEntries = HashSize / sizeof(HashEntry);
	numEntries -= 2;

	if(pTable != nullptr) {
		free(pTable);
	}

	pTable = (HashEntry *) malloc(numEntries * sizeof(HashEntry));
	if(pTable == nullptr) {
		if (MB > 1) {
			printf("Hash Allocation Failed, trying %dMB...\n", MB/2);
			init(MB/2);
		} else {
			printf("ERROR: Hash table allocation failed completely. Cannot allocate even 1MB.\n");
			printf("The engine will run without a transposition table (performance degraded).\n");
			numEntries = 0;
		}
	} else {
		clear();
	}
}

int HashTable::probeEntry(ChessBoard *board, int *move, int *score, int alpha, int beta, int depth) {

	int index = board->posKey % numEntries;

	ASSERT(index >= 0 && index <= numEntries - 1);
	ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
	ASSERT(alpha<beta);
	ASSERT(alpha>=-CHESS_INFINITE&&alpha<=CHESS_INFINITE);
	ASSERT(beta>=-CHESS_INFINITE&&beta<=CHESS_INFINITE);
	ASSERT(board->ply>=0&&board->ply<CHESS_MAX_SEARCH_DEPTH);

	if( pTable[index].posKey == board->posKey ) {
		*move = pTable[index].move;
		if(pTable[index].depth >= depth){
			hit++;

			ASSERT(pTable[index].depth>=1&&pTable[index].depth<CHESS_MAX_SEARCH_DEPTH);
			ASSERT(pTable[index].flags>=HFALPHA&&pTable[index].flags<=HFEXACT);

			*score = pTable[index].score;
			if(*score > CHESS_IS_MATE) *score -= board->ply;
			else if(*score < -CHESS_IS_MATE) *score += board->ply;

			switch(pTable[index].flags) {

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

void HashTable::storeEntry(ChessBoard *board, int move, int score, int flags, int depth) {

	int index = board->posKey % numEntries;

	ASSERT(index >= 0 && index <= numEntries - 1);
	ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
	ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
	ASSERT(score>=-CHESS_INFINITE&&score<=CHESS_INFINITE);
	ASSERT(board->ply>=0&&board->ply<CHESS_MAX_SEARCH_DEPTH);

	if( pTable[index].posKey == 0) {
		newWrite++;
	} else {
		overWrite++;
	}

	if(score > CHESS_IS_MATE) score += board->ply;
	else if(score < -CHESS_IS_MATE) score -= board->ply;

	pTable[index].move = move;
	pTable[index].posKey = board->posKey;
	pTable[index].flags = flags;
	pTable[index].score = score;
	pTable[index].depth = depth;
}

int HashTable::probePvMove(const ChessBoard *board) const {

	int index = board->posKey % numEntries;
	ASSERT(index >= 0 && index <= numEntries - 1);

	if( pTable[index].posKey == board->posKey ) {
		return pTable[index].move;
	}

	return NOMOVE;
}
















