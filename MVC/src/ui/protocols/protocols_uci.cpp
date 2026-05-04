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
#include "../../mvc/IModel.h"
#include <string>

#define INPUTBUFFER 400 * 6

// go depth 6 wtime 180000 btime 100000 binc 1000 winc 1000 movetime 1000 movestogo 40
static void parseGo(char* line, SearchInfo *info, IModel *model, const IEvaluator& eval) {

	ChessBoard *board = model->getBoard();
	if (board == nullptr) {
		return;
	}

	int depth = -1, movestogo = 30,movetime = -1;
	int time = -1, inc = 0;
    char *pointer = NULL;
	info->timeset = BOOL_TYPE_FALSE;

	if ((pointer = strstr(line,"infinite"))) {
		;
	}

	if ((pointer = strstr(line,"binc")) && board->getSide() == COLOR_TYPE_BLACK) {
		inc = atoi(pointer + 5);
	}

	if ((pointer = strstr(line,"winc")) && board->getSide() == COLOR_TYPE_WHITE) {
		inc = atoi(pointer + 5);
	}

	if ((pointer = strstr(line,"wtime")) && board->getSide() == COLOR_TYPE_WHITE) {
		time = atoi(pointer + 6);
	}

	if ((pointer = strstr(line,"btime")) && board->getSide() == COLOR_TYPE_BLACK) {
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

	info->starttime = miscGetTimeMs();
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
	searchPosition(board, info, eval);
}

static void uciLoop(IModel *model, SearchInfo *info, const IEvaluator& eval) {

	info->gameMode = MODE_TYPE_UCI;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	char line[INPUTBUFFER];
    printf("id name %s\n",NAME);
    printf("id author Bluefever\n");
	printf("option name Hash type spin default 64 min 4 max %d\n",CHESS_MAX_HASH);
	printf("option name Book type check default true\n");
    printf("uciok\n");
	
	int mB = 64;

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
            model->applyUciPositionLine(std::string(line));
        } else if (!strncmp(line, "ucinewgame", 10)) {
            model->applyUciPositionLine("position startpos\n");
        } else if (!strncmp(line, "go", 2)) {
            printf("Seen Go..\n");
            parseGo(line, info, model, eval);
        } else if (!strncmp(line, "quit", 4)) {
            info->quit = BOOL_TYPE_TRUE;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",NAME);
            printf("id author Bluefever\n");
            printf("uciok\n");
        } else if (!strncmp(line, "debug", 4)) {
            debugAnalysisTest(model->getBoard(), info, eval);
            break;
        } else if (!strncmp(line, "setoption name Hash value ", 26)) {			
			sscanf(line,"%*s %*s %*s %*s %d",&mB);
			if(mB < 4) mB = 4;
			if(mB > CHESS_MAX_HASH) mB = CHESS_MAX_HASH;
			printf("Set Hash to %d mB\n",mB);
			model->configureHashMegabytes(mB);
		} else if (!strncmp(line, "setoption name Book value ", 26)) {			
			char *ptrTrue = NULL;
			ptrTrue = strstr(line, "true");
			if(ptrTrue != NULL) {
				EngineOptions::instance().setBookEnabled(true);
			} else {
				EngineOptions::instance().setBookEnabled(false);
			}
		}
		if(info->quit) break;
    }
}

void UciProtocol::run(IModel& model, SearchInfo& info, const IEvaluator& eval) {
	uciLoop(&model, &info, eval);
}













