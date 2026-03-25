/**
 * @file protocols_uci.c
 * @brief Universal Chess Interface (UCI) protocol implementation
 * 
 * Implements the UCI protocol for communication with chess GUIs.
 * 
 * Supported UCI commands:
 * - uci: Identify engine
 * - isready: Check if engine is ready
 * - ucinewgame: Start new game
 * - position: Set up position (startpos or fen)
 * - go: Start searching
 * - stop: Stop search
 * - quit: Exit program
 * - setoption: Configure engine options
 * 
 * Reference: http://wbec-ridderkerk.nl/html/UCIProtocol.html
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"
#include "string.h"

#define INPUTBUFFER 400 * 6

// go depth 6 wtime 180000 btime 100000 binc 1000 winc 1000 movetime 1000 movestogo 40
void ParseGo(char* line, SearchInfo *info, ChessBoard *board) {

	int depth = -1, movestogo = 30,movetime = -1;
	int time = -1, inc = 0;
    char *pointer = NULL;
	info->timeset = BOOL_TYPE_FALSE;

	if ((pointer = strstr(line,"infinite"))) {
		;
	}

	if ((pointer = strstr(line,"binc")) && board->side == COLOR_TYPE_BLACK) {
		inc = atoi(pointer + 5);
	}

	if ((pointer = strstr(line,"winc")) && board->side == COLOR_TYPE_WHITE) {
		inc = atoi(pointer + 5);
	}

	if ((pointer = strstr(line,"wtime")) && board->side == COLOR_TYPE_WHITE) {
		time = atoi(pointer + 6);
	}

	if ((pointer = strstr(line,"btime")) && board->side == COLOR_TYPE_BLACK) {
		time = atoi(pointer + 6);
	}

	if ((pointer = strstr(line,"movestogo"))) {
		movestogo = atoi(pointer + 10);
	}

	if ((pointer = strstr(line,"movetime"))) {
		movetime = atoi(pointer + 9);
	}

	if ((pointer = strstr(line,"depth"))) {
		depth = atoi(pointer + 6);
	}

	if(movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = Misc_GetTimeMs();
	info->depth = depth;

	if(time != -1) {
		info->timeset = BOOL_TYPE_TRUE;
		time /= movestogo;
		time -= 50;
		info->stoptime = info->starttime + time + inc;
	}

	if(depth == -1) {
		info->depth = CHESS_MAX_SEARCH_DEPTH;
	}

	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		time,info->starttime,info->stoptime,info->depth,info->timeset);
	Search_Position(board, info);
}

// position fen fenstr
// position startpos
// ... moves e2e4 e7e5 b7b8q
void ParsePosition(char* lineIn, ChessBoard *board) {

	lineIn += 9;
    char *ptrChar = lineIn;

    if(strncmp(lineIn, "startpos", 8) == 0){
        Board_ParseFromFEN(CHESS_START_FEN, board);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if(ptrChar == NULL) {
            Board_ParseFromFEN(CHESS_START_FEN, board);
        } else {
            ptrChar+=4;
            Board_ParseFromFEN(ptrChar, board);
        }
    }

	ptrChar = strstr(lineIn, "moves");
	int move;

	if(ptrChar != NULL) {
        ptrChar += 6;
        while(*ptrChar) {
              move = Move_Parse(ptrChar,board);
			  if(move == NOMOVE) break;
			  Move_Make(board, move);
              board->ply=0;
              while(*ptrChar && *ptrChar!= ' ') ptrChar++;
              ptrChar++;
        }
    }
	Board_Print(board);
}

void Uci_Loop(ChessBoard *board, SearchInfo *info) {

	info->GAME_MODE = MODE_TYPE_UCI;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	char line[INPUTBUFFER];
    printf("id name %s\n",NAME);
    printf("id author Bluefever\n");
	printf("option name Hash type spin default 64 min 4 max %d\n",CHESS_MAX_HASH);
	printf("option name Book type check default true\n");
    printf("uciok\n");
	
	int MB = 64;

	while (BOOL_TYPE_TRUE) {
		memset(&line[0], 0, sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin))
        continue;

        if (line[0] == '\n')
        continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "position", 8)) {
            ParsePosition(line, board);
        } else if (!strncmp(line, "ucinewgame", 10)) {
            ParsePosition("position startpos\n", board);
        } else if (!strncmp(line, "go", 2)) {
            printf("Seen Go..\n");
            ParseGo(line, info, board);
        } else if (!strncmp(line, "quit", 4)) {
            info->quit = BOOL_TYPE_TRUE;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",NAME);
            printf("id author Bluefever\n");
            printf("uciok\n");
        } else if (!strncmp(line, "debug", 4)) {
            DebugAnalysisTest(board,info);
            break;
        } else if (!strncmp(line, "setoption name Hash value ", 26)) {			
			sscanf(line,"%*s %*s %*s %*s %d",&MB);
			if(MB < 4) MB = 4;
			if(MB > CHESS_MAX_HASH) MB = CHESS_MAX_HASH;
			printf("Set Hash to %d MB\n",MB);
			HashTable_Init(board->HashTable, MB);
		} else if (!strncmp(line, "setoption name Book value ", 26)) {			
			char *ptrTrue = NULL;
			ptrTrue = strstr(line, "true");
			if(ptrTrue != NULL) {
				EngineOptions->UseBook = BOOL_TYPE_TRUE;
			} else {
				EngineOptions->UseBook = BOOL_TYPE_FALSE;
			}
		}
		if(info->quit) break;
    }
}













