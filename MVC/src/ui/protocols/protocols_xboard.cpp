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
#include "../../mvc/IModel.h"
#include <string>
#include <cstddef>

namespace {

ChessBoard *boardOrNull(IModel *m) {
	return m ? m->getBoard() : nullptr;
}

void extractFirstToken(const char *src, char *out, size_t cap) {
	if (!src || !out || cap == 0) {
		return;
	}
	while (*src == ' ' || *src == '\t') {
		src++;
	}
	size_t i = 0;
	while (*src && *src != ' ' && *src != '\n' && *src != '\r' && i + 1 < cap) {
		out[i++] = *src++;
	}
	out[i] = '\0';
}

std::string fenPayloadTrimmed(const char *inBuf, size_t skipChars) {
	std::string s(inBuf + skipChars);
	while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
		s.pop_back();
	}
	size_t st = 0;
	while (st < s.size() && (s[st] == ' ' || s[st] == '\t')) {
		st++;
	}
	return s.substr(st);
}

} // namespace

void printOptions() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
	printf("feature done=1\n");
}

static void xBoardLoop(IModel *model, SearchInfo *info, const IEvaluator& eval) {

	ChessBoard *board = boardOrNull(model);
	if (board == nullptr) {
		return;
	}

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
			model->configureHashMegabytes(mB);
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
			model->protocolXBoardNewGame();
			engineSide = COLOR_TYPE_BLACK;
			depth = -1;
			time = -1;
			continue;
		}

		if(!strcmp(command, "setboard")){
			engineSide = COLOR_TYPE_BOTH;
			model->setBoardFromFenPayload(fenPayloadTrimmed(inBuf, 9));
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = board->getSide();
			continue;
		}

		if(!strcmp(command, "usermove")){
			movestogo[board->getSide()]--;
			char token[80];
			extractFirstToken(inBuf + 9, token, sizeof(token));
			if(token[0]=='\0') continue;
			if(!model->applyUciCoordToken(token)) continue;
			model->resetSearchRootPly();
		}
    }
}

void XBoardProtocol::run(IModel& model, SearchInfo& info, const IEvaluator& eval) {
	xBoardLoop(&model, &info, eval);
}




















