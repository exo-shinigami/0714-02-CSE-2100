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

void printOptions() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
	printf("feature done=1\n");
}

void xBoardLoop(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {

	info->gameMode = MODE_TYPE_XBOARD;
	info->postThinking = BOOL_TYPE_TRUE;
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	printOptions(); // Advertise supported XBoard features on startup.

	int depth = -1, movestogo[2] = {30,30 }, movetime = -1;
	int time = -1, inc = 0;
	int engineSide = COLOR_TYPE_BOTH;
	int timeLeft;
	int sec;
	int mps;
	int move = NOMOVE;
	char inBuf[80], command[80];
	int mB;

	engineSide = COLOR_TYPE_BLACK;
	board->parseFromFEN(CHESS_START_FEN);
	depth = -1;
	time = -1;

	while(BOOL_TYPE_TRUE) {

		fflush(stdout);

		if(board->getSide() == engineSide && checkresult(board) == BOOL_TYPE_FALSE) {
			info->starttime = miscGetTimeMs();
			info->depth = depth;

			if(time != -1) {
				info->timeset = BOOL_TYPE_TRUE;
				time /= movestogo[board->getSide()];
				time -= 50;
				info->stoptime = info->starttime + time + inc;
			}

			if(depth == -1 || depth > CHESS_MAX_SEARCH_DEPTH) {
				info->depth = CHESS_MAX_SEARCH_DEPTH;
			}

			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time,info->starttime,info->stoptime,info->depth,info->timeset, movestogo[board->getSide()], mps);
				searchPosition(board, info, eval);

			if(mps != 0) {
				movestogo[board->getSide()^1]--;
				if(movestogo[board->getSide()^1] < 1) {
					movestogo[board->getSide()^1] = mps;
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
			printOptions();
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
			sscanf(inBuf, "memory %d", &mB);		
		    if(mB < 4) mB = 4;
			if(mB > CHESS_MAX_HASH) mB = CHESS_MAX_HASH;
			printf("Set Hash to %d mB\n",mB);
			board->initHashTable(mB);
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
			board->clearHashTable();
			engineSide = COLOR_TYPE_BLACK;
			board->parseFromFEN(CHESS_START_FEN);
			depth = -1;
			time = -1;
			continue;
		}

		if(!strcmp(command, "setboard")){
			engineSide = COLOR_TYPE_BOTH;
			board->parseFromFEN(inBuf+9);
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = board->getSide();
			continue;
		}

		if(!strcmp(command, "usermove")){
			movestogo[board->getSide()]--;
			move = moveParse(inBuf+9, board);
			if(move == NOMOVE) continue;
			board->makeMove(move);
			board->setPly(0);
		}
    }
}


void consoleLoop(ChessBoard *board, SearchInfo *info, const IEvaluator& eval) {

	printf("Welcome to Gambit In Console Mode!\n");
	printf("Type help for commands\n\n");

	info->gameMode = MODE_TYPE_CONSOLE;
	info->postThinking = BOOL_TYPE_TRUE;
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	int depth = CHESS_MAX_SEARCH_DEPTH, movetime = 3000;
	int engineSide = COLOR_TYPE_BOTH;
	int move = NOMOVE;
	char inBuf[80], command[80];

	engineSide = COLOR_TYPE_BLACK;
	board->parseFromFEN(CHESS_START_FEN);

	while(BOOL_TYPE_TRUE) {

		fflush(stdout);

		if(board->getSide() == engineSide && checkresult(board) == BOOL_TYPE_FALSE) {
			info->starttime = miscGetTimeMs();
			info->depth = depth;

			if(movetime != 0) {
				info->timeset = BOOL_TYPE_TRUE;
				info->stoptime = info->starttime + movetime;
			}

			searchPosition(board, info, eval);
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
			mirrorEvalTest(board);
			continue;
		}

		if(!strcmp(command, "eval")) {
			board->print();
			printf("Eval:%d", eval.evaluate(*board));
			board->mirror();
			board->print();
			printf("Eval:%d", eval.evaluate(*board));
			continue;
		}

		if(!strcmp(command, "setboard")){
			engineSide = COLOR_TYPE_BOTH;
			board->parseFromFEN(inBuf+9);
			continue;
		}

		if(!strcmp(command, "quit")) {
			info->quit = BOOL_TYPE_TRUE;
			break;
		}

		if(!strcmp(command, "post")) {
			info->postThinking = BOOL_TYPE_TRUE;
			continue;
		}

		if(!strcmp(command, "print")) {
			board->print();
			continue;
		}

		if(!strcmp(command, "nopost")) {
			info->postThinking = BOOL_TYPE_FALSE;
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
			board->clearHashTable();
			engineSide = COLOR_TYPE_BLACK;
			board->parseFromFEN(CHESS_START_FEN);
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = board->getSide();
			continue;
		}

		move = moveParse(inBuf, board);
		if(move == NOMOVE) {
			printf("Command unknown:%s\n",inBuf);
			continue;
		}
		board->makeMove(move);
		board->setPly(0);
    }
}

void XBoardProtocol::run(ChessBoard& board, SearchInfo& info, const IEvaluator& eval) {
	xBoardLoop(&board, &info, eval);
}




















