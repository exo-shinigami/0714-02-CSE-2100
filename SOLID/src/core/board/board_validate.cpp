/**
 * @file board_validate.c
 * @brief Validation and debugging functions
 * 
 * Provides various validation functions to check the integrity of:
 * - Square indices (on-board, 120-square format)
 * - Piece values
 * - Side values
 * - File and rank indices
 * - Move lists
 * 
 * Also includes debugging functions:
 * - Evaluation symmetry testing
 * - Analysis testing
 * 
 * These functions are primarily used in DEBUG mode to catch
 * programming errors early in development.
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "types_definitions.h"
#include "stdio.h"
#include "string.h"

int MoveListOk(const MoveList *list,  const ChessBoard *board) {
	if(list->size() < 0 || list->size() >= CHESS_MAX_POSITION_MOVES) {
		return BOOL_TYPE_FALSE;
	}

	int MoveNum;
	int from = 0;
	int to = 0;
	for(MoveNum = 0; MoveNum < list->size(); ++MoveNum) {
		to = MOVE_GET_TO_SQUARE(list->at(MoveNum).raw());
		from = MOVE_GET_FROM_SQUARE(list->at(MoveNum).raw());
		if(!SqOnBoard(to) || !SqOnBoard(from)) {
			return BOOL_TYPE_FALSE;
		}
		if(!PieceValid(board->pieces[from])) {
			board->print();
			return BOOL_TYPE_FALSE;
		}
	}

	return BOOL_TYPE_TRUE;
}

int SqIs120(const int squareIndex) {
	return (squareIndex>=0 && squareIndex<120);
}

int PceValidEmptyOffbrd(const int piece) {
	return (PieceValidEmpty(piece) || piece == OFFBOARD);
}
int SqOnBoard(const int squareIndex) {
	return g_filesBoard[squareIndex]==OFFBOARD ? 0 : 1;
}

int SideValid(const int side) {
	return (side==COLOR_TYPE_WHITE || side == COLOR_TYPE_BLACK) ? 1 : 0;
}

int FileRankValid(const int fr) {
	return (fr >= 0 && fr <= 7) ? 1 : 0;
}

int PieceValidEmpty(const int piece) {
	return (piece >= EMPTY && piece <= PIECE_TYPE_BLACK_KING) ? 1 : 0;
}

int PieceValid(const int piece) {
	return (piece >= PIECE_TYPE_WHITE_PAWN && piece <= PIECE_TYPE_BLACK_KING) ? 1 : 0;
}

void DebugAnalysisTest(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {

	FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];

	info->depth = CHESS_MAX_SEARCH_DEPTH;
	info->timeset = BOOL_TYPE_TRUE;
	int time = 1140000;


    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
			info->starttime = Misc_GetTimeMs();
			info->stoptime = info->starttime + time;
			board->hashTable.clear();
            board->parseFromFEN(lineIn);
            printf("\n%s\n",lineIn);
			printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
				time,info->starttime,info->stoptime,info->depth,info->timeset);
			Search_Position(board, info, eval);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}



void MirrorEvalTest(ChessBoard *board) {
    FILE *file;
    file = fopen("mirror.epd","r");
    char lineIn [1024];
    int ev1 = 0; int ev2 = 0;
    int positions = 0;
    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
            board->parseFromFEN(lineIn);
            positions++;
            ev1 = Evaluate_Position(board);
            board->mirror();
            ev2 = Evaluate_Position(board);

            if(ev1 != ev2) {
                printf("\n\n\n");
                board->parseFromFEN(lineIn);
                board->print();
                board->mirror();
                board->print();
                printf("\n\nMirror Fail:\n%s\n",lineIn);
                getchar();
                return;
            }

            if( (positions % 1000) == 0)   {
                printf("position %d\n",positions);
            }

            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}

