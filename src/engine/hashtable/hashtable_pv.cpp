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

		if( moveExists(board, move) ) {
			board->makeMove(move);
			info->setPvMove(count++, move);
		} else {
			break;
		}
		move = probePvMove(board);
	}

	while(board->getPly() > 0) {
		board->takeMove();
	}

	return count;
}

void HashTable::clear() {
	for (HashEntry& tableEntry : table_) {
		tableEntry.posKey = 0ULL;
		tableEntry.move = NOMOVE;
		tableEntry.depth = 0;
		tableEntry.score = 0;
		tableEntry.flags = 0;
	}
	newWrite = 0;
}

void HashTable::init(int mB) {

	int hashSize = 0x100000 * mB;
	numEntries = hashSize / sizeof(HashEntry);
	numEntries -= 2;

	if (numEntries < 1) {
		numEntries = 0;
		table_.clear();
		return;
	}

	try {
		table_.assign(numEntries, HashEntry{});
		clear();
	} catch (...) {
		if (mB > 1) {
			printf("Hash Allocation Failed, trying %dMB...\n", mB/2);
			init(mB/2);
		} else {
			printf("ERROR: Hash table allocation failed completely. Cannot allocate even 1MB.\n");
			printf("The engine will run without a transposition table (performance degraded).\n");
			numEntries = 0;
			table_.clear();
		}
	}
}

int HashTable::probeEntry(ChessBoard *board, int *move, int *score, int alpha, int beta, int depth) {
	if (numEntries <= 0) {
		return BOOL_TYPE_FALSE;
	}

	int index = board->getPositionKey() % numEntries;

	ASSERT(index >= 0 && index <= numEntries - 1);
	ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
	ASSERT(alpha<beta);
	ASSERT(alpha>=-CHESS_INFINITE&&alpha<=CHESS_INFINITE);
	ASSERT(beta>=-CHESS_INFINITE&&beta<=CHESS_INFINITE);
	ASSERT(board->getPly()>=0&&board->getPly()<CHESS_MAX_SEARCH_DEPTH);

	if( table_[index].posKey == board->getPositionKey() ) {
		*move = table_[index].move;
		if(table_[index].depth >= depth){
			hit++;

			ASSERT(table_[index].depth>=1&&table_[index].depth<CHESS_MAX_SEARCH_DEPTH);
			ASSERT(table_[index].flags>=HFALPHA&&table_[index].flags<=HFEXACT);

			*score = table_[index].score;
			if(*score > CHESS_IS_MATE) *score -= board->getPly();
			else if(*score < -CHESS_IS_MATE) *score += board->getPly();

			switch(table_[index].flags) {

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
	if (numEntries <= 0) {
		return;
	}

	int index = board->getPositionKey() % numEntries;

	ASSERT(index >= 0 && index <= numEntries - 1);
	ASSERT(depth>=1&&depth<CHESS_MAX_SEARCH_DEPTH);
	ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
	ASSERT(score>=-CHESS_INFINITE&&score<=CHESS_INFINITE);
	ASSERT(board->getPly()>=0&&board->getPly()<CHESS_MAX_SEARCH_DEPTH);

	if( table_[index].posKey == 0) {
		newWrite++;
	} else {
		overWrite++;
	}

	if(score > CHESS_IS_MATE) score += board->getPly();
	else if(score < -CHESS_IS_MATE) score -= board->getPly();

	table_[index].move = move;
	table_[index].posKey = board->getPositionKey();
	table_[index].flags = flags;
	table_[index].score = score;
	table_[index].depth = depth;
}

int HashTable::probePvMove(const ChessBoard *board) const {
	if (numEntries <= 0) {
		return NOMOVE;
	}

	int index = board->getPositionKey() % numEntries;
	ASSERT(index >= 0 && index <= numEntries - 1);

	if( table_[index].posKey == board->getPositionKey() ) {
		return table_[index].move;
	}

	return NOMOVE;
}
















