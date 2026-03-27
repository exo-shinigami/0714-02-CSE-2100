/**
 * @file board_validate.c
 * @brief Validation and debugging functions
 * 
 * Provides various validation functions to check the integrity of:
 * - Square indices (on-board, 120-square format)
 * - Piece values
 * - Side values
 * - File and rank indices
 * - move lists
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

int ValidationService::isMoveListValid(const MoveList& list, const ChessBoard& board) {
    if(list.size() < 0 || list.size() >= CHESS_MAX_POSITION_MOVES) {
		return BOOL_TYPE_FALSE;
	}

	int moveNum;
	int from = 0;
	int to = 0;
    for(moveNum = 0; moveNum < list.size(); ++moveNum) {
        to = MOVE_GET_TO_SQUARE(list.at(moveNum).raw());
        from = MOVE_GET_FROM_SQUARE(list.at(moveNum).raw());
        if(!isSquareOnBoard(to) || !isSquareOnBoard(from)) {
			return BOOL_TYPE_FALSE;
		}
        if(!isPieceValid(board.pieceAt(from))) {
            board.print();
			return BOOL_TYPE_FALSE;
		}
	}

	return BOOL_TYPE_TRUE;
}

int ValidationService::isSquare120(int squareIndex) {
	return (squareIndex>=0 && squareIndex<120);
}

int ValidationService::isPieceValidEmptyOrOffboard(int piece) {
    return (isPieceValidEmpty(piece) || piece == OFFBOARD);
}

int ValidationService::isSquareOnBoard(int squareIndex) {
	return g_filesBoard[squareIndex]==OFFBOARD ? 0 : 1;
}

int ValidationService::isSideValid(int side) {
	return (side==COLOR_TYPE_WHITE || side == COLOR_TYPE_BLACK) ? 1 : 0;
}

int ValidationService::isFileRankValid(int fileOrRank) {
    return (fileOrRank >= 0 && fileOrRank <= 7) ? 1 : 0;
}

int ValidationService::isPieceValidEmpty(int piece) {
	return (piece >= EMPTY && piece <= PIECE_TYPE_BLACK_KING) ? 1 : 0;
}

int ValidationService::isPieceValid(int piece) {
	return (piece >= PIECE_TYPE_WHITE_PAWN && piece <= PIECE_TYPE_BLACK_KING) ? 1 : 0;
}

void ValidationService::runAnalysisTest(ChessBoard& board, SearchInfo& info, const IEvaluator& eval) {

	FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];

    info.depth = CHESS_MAX_SEARCH_DEPTH;
    info.timeset = BOOL_TYPE_TRUE;
	int time = 1140000;


    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
			info.starttime = miscGetTimeMs();
			info.stoptime = info.starttime + time;
            board.clearHashTable();
            board.parseFromFEN(lineIn);
            printf("\n%s\n",lineIn);
			printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
				time,info.starttime,info.stoptime,info.depth,info.timeset);
			searchPosition(&board, &info, eval);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}



void ValidationService::runMirrorEvalTest(ChessBoard& board) {
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
            board.parseFromFEN(lineIn);
            positions++;
            ev1 = evaluatePosition(&board);
            board.mirror();
            ev2 = evaluatePosition(&board);

            if(ev1 != ev2) {
                printf("\n\n\n");
                board.parseFromFEN(lineIn);
                board.print();
                board.mirror();
                board.print();
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

int moveListOk(const MoveList *list, const ChessBoard *board) {
    ASSERT(list != nullptr && board != nullptr);
    return ValidationService::isMoveListValid(*list, *board);
}

int sqIs120(const int squareIndex) {
    return ValidationService::isSquare120(squareIndex);
}

int pceValidEmptyOffbrd(const int piece) {
    return ValidationService::isPieceValidEmptyOrOffboard(piece);
}

int sqOnBoard(const int squareIndex) {
    return ValidationService::isSquareOnBoard(squareIndex);
}

int sideValid(const int side) {
    return ValidationService::isSideValid(side);
}

int fileRankValid(const int fr) {
    return ValidationService::isFileRankValid(fr);
}

int pieceValidEmpty(const int piece) {
    return ValidationService::isPieceValidEmpty(piece);
}

int pieceValid(const int piece) {
    return ValidationService::isPieceValid(piece);
}

void debugAnalysisTest(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {
    ASSERT(board != nullptr && info != nullptr);
    ValidationService::runAnalysisTest(*board, *info, eval);
}

void mirrorEvalTest(ChessBoard *board) {
    ASSERT(board != nullptr);
    ValidationService::runMirrorEvalTest(*board);
}

