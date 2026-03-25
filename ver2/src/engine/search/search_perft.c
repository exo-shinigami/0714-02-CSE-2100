/**
 * @file search_perft.c
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

#include "types_definitions.h"
#include "stdio.h"

long leafNodes;

void Perft(int depth, ChessBoard *board) {

    ASSERT(Board_Check(board));  

	if(depth == 0) {
        leafNodes++;
        return;
    }	

    MoveList list[1];
    Move_GenerateAll(board,list);
      
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
       
        if ( !Move_Make(board,list->moves[MoveNum].move))  {
            continue;
        }
        Perft(depth - 1, board);
        Move_Take(board);
    }

    return;
}


void Search_PerftTest(int depth, ChessBoard *board) {

    ASSERT(Board_Check(board));

	Board_Print(board);
	printf("\nStarting Test To Depth:%d\n",depth);	
	leafNodes = 0;
	int start = Misc_GetTimeMs();
    MoveList list[1];
    Move_GenerateAll(board,list);	
    
    int move;	    
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if ( !Move_Make(board,move))  {
            continue;
        }
        long cumnodes = leafNodes;
        Perft(depth - 1, board);
        Move_Take(board);        
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n",MoveNum+1,PrMove(move),oldnodes);
    }
	
	printf("\nTest Complete : %ld nodes visited in %dms\n",leafNodes,Misc_GetTimeMs() - start);

    return;
}












