/**
 * @file perft.c
 * @brief Performance testing (Perft) for move generation validation
 * 
 * Perft (Performance Test) is a debugging function that counts all
 * leaf nodes at a given depth. It's used to:
 * - Validate move generation correctness
 * - Test make/unmake move functions
 * - Benchmark move generation speed
 * - Compare with known perft values from test positions
 * 
 * The function performs an exhaustive tree search to a specified depth
 * and counts all positions reached, dividing by root move.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "defs.h"
#include "stdio.h"

long leafNodes;

void Perft(int depth, S_BOARD *pos) {

    ASSERT(CheckBoard(pos));  

	if(depth == 0) {
        leafNodes++;
        return;
    }	

    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);
      
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
       
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        Perft(depth - 1, pos);
        TakeMove(pos);
    }

    return;
}


void PerftTest(int depth, S_BOARD *pos) {

    ASSERT(CheckBoard(pos));

	PrintBoard(pos);
	printf("\nStarting Test To Depth:%d\n",depth);	
	leafNodes = 0;
	int start = GetTimeMs();
    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);	
    
    int move;	    
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if ( !MakeMove(pos,move))  {
            continue;
        }
        long cumnodes = leafNodes;
        Perft(depth - 1, pos);
        TakeMove(pos);        
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n",MoveNum+1,PrMove(move),oldnodes);
    }
	
	printf("\nTest Complete : %ld nodes visited in %dms\n",leafNodes,GetTimeMs() - start);

    return;
}












