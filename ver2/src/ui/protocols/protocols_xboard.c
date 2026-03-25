/**
 * @file protocols_xboard.c
 * @brief XBoard/WinBoard protocol and console mode implementation
 * 
 * Implements:
 * - XBoard/WinBoard protocol for chess GUIs
 * - Console mode for human vs computer play
 * - Game state management (checkmate, stalemate, draws)
 * - Time control
 * 
 * Supported XBoard commands:
 * - new, force, go, usermove
 * - time, otim, level
 * - quit, post, nopost
 * - And more standard XBoard protocol commands
 * 
 * Also includes draw detection:
 * - Fifty-move rule
 * - Threefold repetition
 * - Insufficient material
 * - Stalemate
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"
#include "string.h"

int ThreeFoldRep(const ChessBoard *board) {

	ASSERT(Board_Check(board));

	int i = 0, r = 0;
	for (i = 0; i < board->hisPly; ++i)	{
	    if (board->history[i].posKey == board->posKey) {
		    r++;
		}
	}
	return r;
}

int DrawMaterial(const ChessBoard *board) {
	ASSERT(Board_Check(board));

    if (board->pieceCount[PIECE_TYPE_WHITE_PAWN] || board->pieceCount[PIECE_TYPE_BLACK_PAWN]) return BOOL_TYPE_FALSE;
    if (board->pieceCount[PIECE_TYPE_WHITE_QUEEN] || board->pieceCount[PIECE_TYPE_BLACK_QUEEN] || board->pieceCount[PIECE_TYPE_WHITE_ROOK] || board->pieceCount[PIECE_TYPE_BLACK_ROOK]) return BOOL_TYPE_FALSE;
    if (board->pieceCount[PIECE_TYPE_WHITE_BISHOP] > 1 || board->pieceCount[PIECE_TYPE_BLACK_BISHOP] > 1) {return BOOL_TYPE_FALSE;}
    if (board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] > 1 || board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] > 1) {return BOOL_TYPE_FALSE;}
    if (board->pieceCount[PIECE_TYPE_WHITE_KNIGHT] && board->pieceCount[PIECE_TYPE_WHITE_BISHOP]) {return BOOL_TYPE_FALSE;}
    if (board->pieceCount[PIECE_TYPE_BLACK_KNIGHT] && board->pieceCount[PIECE_TYPE_BLACK_BISHOP]) {return BOOL_TYPE_FALSE;}

    return BOOL_TYPE_TRUE;
}

int checkresult(ChessBoard *board) {
	ASSERT(Board_Check(board));

    if (board->fiftyMove > 100) {
     printf("1/2-1/2 {fifty move rule (claimed by Gambit)}\n"); return BOOL_TYPE_TRUE;
    }

    if (ThreeFoldRep(board) >= 2) {
     printf("1/2-1/2 {3-fold repetition (claimed by Gambit)}\n"); return BOOL_TYPE_TRUE;
    }

	if (DrawMaterial(board) == BOOL_TYPE_TRUE) {
     printf("1/2-1/2 {insufficient material (claimed by Gambit)}\n"); return BOOL_TYPE_TRUE;
    }

	MoveList list[1];
    Move_GenerateAll(board,list);

    int MoveNum = 0;
	int found = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if ( !Move_Make(board,list->moves[MoveNum].move))  {
            continue;
        }
        found++;
		Move_Take(board);
		break;
    }

	if(found != 0) return BOOL_TYPE_FALSE;

	int InCheck = Attack_IsSquareAttacked(board->KingSq[board->side],board->side^1,board);

	if(InCheck == BOOL_TYPE_TRUE)	{
	    if(board->side == COLOR_TYPE_WHITE) {
	      printf("0-1 {black mates (claimed by Gambit)}\n");return BOOL_TYPE_TRUE;
        } else {
	      printf("1-0 {white mates (claimed by Gambit)}\n");return BOOL_TYPE_TRUE;
        }
    } else {
      printf("\n1/2-1/2 {stalemate (claimed by Gambit)}\n");return BOOL_TYPE_TRUE;
    }
	return BOOL_TYPE_FALSE;
}

void PrintOptions() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
	printf("feature done=1\n");
}

void XBoard_Loop(ChessBoard *board, SearchInfo *info) {

	info->GAME_MODE = MODE_TYPE_XBOARD;
	info->POST_THINKING = BOOL_TYPE_TRUE;
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	PrintOptions(); // HACK

	int depth = -1, movestogo[2] = {30,30 }, movetime = -1;
	int time = -1, inc = 0;
	int engineSide = COLOR_TYPE_BOTH;
	int timeLeft;
	int sec;
	int mps;
	int move = NOMOVE;
	char inBuf[80], command[80];
	int MB;

	engineSide = COLOR_TYPE_BLACK;
	Board_ParseFromFEN(CHESS_START_FEN, board);
	depth = -1;
	time = -1;

	while(BOOL_TYPE_TRUE) {

		fflush(stdout);

		if(board->side == engineSide && checkresult(board) == BOOL_TYPE_FALSE) {
			info->starttime = Misc_GetTimeMs();
			info->depth = depth;

			if(time != -1) {
				info->timeset = BOOL_TYPE_TRUE;
				time /= movestogo[board->side];
				time -= 50;
				info->stoptime = info->starttime + time + inc;
			}

			if(depth == -1 || depth > CHESS_MAX_SEARCH_DEPTH) {
				info->depth = CHESS_MAX_SEARCH_DEPTH;
			}

			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time,info->starttime,info->stoptime,info->depth,info->timeset, movestogo[board->side], mps);
				Search_Position(board, info);

			if(mps != 0) {
				movestogo[board->side^1]--;
				if(movestogo[board->side^1] < 1) {
					movestogo[board->side^1] = mps;
				}
			}

		}

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);
		if (!fgets(inBuf, 80, stdin))
		continue;

		sscanf(inBuf, "%s", command);

		printf("command seen:%s\n",inBuf);

		if(!strcmp(command, "quit")) {
			info->quit = BOOL_TYPE_TRUE;
			break;
		}

		if(!strcmp(command, "force")) {
			engineSide = COLOR_TYPE_BOTH;
			continue;
		}

		if(!strcmp(command, "protover")){
			PrintOptions();
		    continue;
		}

		if(!strcmp(command, "sd")) {
			sscanf(inBuf, "sd %d", &depth);
		    printf("DEBUG depth:%d\n",depth);
			continue;
		}

		if(!strcmp(command, "st")) {
			sscanf(inBuf, "st %d", &movetime);
		    printf("DEBUG movetime:%d\n",movetime);
			continue;
		}

		if(!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &time);
			time *= 10;
		    printf("DEBUG time:%d\n",time);
			continue;
		}
		
		if(!strcmp(command, "memory")) {			
			sscanf(inBuf, "memory %d", &MB);		
		    if(MB < 4) MB = 4;
			if(MB > CHESS_MAX_HASH) MB = CHESS_MAX_HASH;
			printf("Set Hash to %d MB\n",MB);
			HashTable_Init(board->HashTable, MB);
			continue;
		}

		if(!strcmp(command, "level")) {
			sec = 0;
			movetime = -1;
			if( sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
			  sscanf(inBuf, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
		      printf("DEBUG level with :\n");
			}	else {
		      printf("DEBUG level without :\n");
			}
			timeLeft *= 60000;
			timeLeft += sec * 1000;
			movestogo[0] = movestogo[1] = 30;
			if(mps != 0) {
				movestogo[0] = movestogo[1] = mps;
			}
			time = -1;
		    printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n",timeLeft,movestogo[0],inc,mps);
			continue;
		}

		if(!strcmp(command, "ping")) {
			printf("pong%s\n", inBuf+4);
			continue;
		}

		if(!strcmp(command, "new")) {
			HashTable_Clear(board->HashTable);
			engineSide = COLOR_TYPE_BLACK;
			Board_ParseFromFEN(CHESS_START_FEN, board);
			depth = -1;
			time = -1;
			continue;
		}

		if(!strcmp(command, "setboard")){
			engineSide = COLOR_TYPE_BOTH;
			Board_ParseFromFEN(inBuf+9, board);
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = board->side;
			continue;
		}

		if(!strcmp(command, "usermove")){
			movestogo[board->side]--;
			move = Move_Parse(inBuf+9, board);
			if(move == NOMOVE) continue;
			Move_Make(board, move);
            board->ply=0;
		}
    }
}


void Console_Loop(ChessBoard *board, SearchInfo *info) {

	printf("Welcome to Gambit In Console Mode!\n");
	printf("Type help for commands\n\n");

	info->GAME_MODE = MODE_TYPE_CONSOLE;
	info->POST_THINKING = BOOL_TYPE_TRUE;
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	int depth = CHESS_MAX_SEARCH_DEPTH, movetime = 3000;
	int engineSide = COLOR_TYPE_BOTH;
	int move = NOMOVE;
	char inBuf[80], command[80];

	engineSide = COLOR_TYPE_BLACK;
	Board_ParseFromFEN(CHESS_START_FEN, board);

	while(BOOL_TYPE_TRUE) {

		fflush(stdout);

		if(board->side == engineSide && checkresult(board) == BOOL_TYPE_FALSE) {
			info->starttime = Misc_GetTimeMs();
			info->depth = depth;

			if(movetime != 0) {
				info->timeset = BOOL_TYPE_TRUE;
				info->stoptime = info->starttime + movetime;
			}

			Search_Position(board, info);
		}

		printf("\nGambit > ");

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);
		if (!fgets(inBuf, 80, stdin))
		continue;

		sscanf(inBuf, "%s", command);

		if(!strcmp(command, "help")) {
			printf("Commands:\n");
			printf("quit - quit game\n");
			printf("force - computer will not think\n");
			printf("print - show board\n");
			printf("post - show thinking\n");
			printf("nopost - do not show thinking\n");
			printf("new - start new game\n");
			printf("go - set computer thinking\n");
			printf("depth x - set depth to x\n");
			printf("time x - set thinking time to x seconds (depth still applies if set)\n");
			printf("view - show current depth and movetime settings\n");
			printf("setboard x - set position to fen x\n");
			printf("** note ** - to reset time and depth, set to 0\n");
			printf("enter moves using b7b8q notation\n\n\n");
			continue;
		}

		if(!strcmp(command, "mirror")) {
			engineSide = COLOR_TYPE_BOTH;
			MirrorEvalTest(board);
			continue;
		}

		if(!strcmp(command, "eval")) {
			Board_Print(board);
			printf("Eval:%d",Evaluate_Position(board));
			Board_Mirror(board);
			Board_Print(board);
			printf("Eval:%d",Evaluate_Position(board));
			continue;
		}

		if(!strcmp(command, "setboard")){
			engineSide = COLOR_TYPE_BOTH;
			Board_ParseFromFEN(inBuf+9, board);
			continue;
		}

		if(!strcmp(command, "quit")) {
			info->quit = BOOL_TYPE_TRUE;
			break;
		}

		if(!strcmp(command, "post")) {
			info->POST_THINKING = BOOL_TYPE_TRUE;
			continue;
		}

		if(!strcmp(command, "print")) {
			Board_Print(board);
			continue;
		}

		if(!strcmp(command, "nopost")) {
			info->POST_THINKING = BOOL_TYPE_FALSE;
			continue;
		}

		if(!strcmp(command, "force")) {
			engineSide = COLOR_TYPE_BOTH;
			continue;
		}

		if(!strcmp(command, "view")) {
			if(depth == CHESS_MAX_SEARCH_DEPTH) printf("depth not set ");
			else printf("depth %d",depth);

			if(movetime != 0) printf(" movetime %ds\n",movetime/1000);
			else printf(" movetime not set\n");

			continue;
		}

		if(!strcmp(command, "depth")) {
			sscanf(inBuf, "depth %d", &depth);
		    if(depth==0) depth = CHESS_MAX_SEARCH_DEPTH;
			continue;
		}

		if(!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &movetime);
			movetime *= 1000;
			continue;
		}

		if(!strcmp(command, "new")) {
			HashTable_Clear(board->HashTable);
			engineSide = COLOR_TYPE_BLACK;
			Board_ParseFromFEN(CHESS_START_FEN, board);
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = board->side;
			continue;
		}

		move = Move_Parse(inBuf, board);
		if(move == NOMOVE) {
			printf("Command unknown:%s\n",inBuf);
			continue;
		}
		Move_Make(board, move);
		board->ply=0;
    }
}




















