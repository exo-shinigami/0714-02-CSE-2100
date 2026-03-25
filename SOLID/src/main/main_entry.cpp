/**
 * @file main_entry.c
 * @brief Main entry point for Gambit Chess Engine
 * 
 * This file contains the main() function and handles:
 * - Engine initialization
 * - Command-line argument parsing
 * - GUI mode launch (default)
 * - Protocol modes (UCI, XBoard) via command line
 * - Program startup and shutdown
 * 
 * Usage:
 *   gambit           - Launch GUI mode (default)
 *   gambit uci       - Launch UCI protocol mode
 *   gambit xboard    - Launch XBoard protocol mode
 *   gambit NoBook    - Launch GUI with opening book disabled
 * 
 * @author Gambit Chess Team
 * @date February 2026
 * @version 2.0
 */

#include "stdio.h"
#include "types_definitions.h"
#ifdef ENABLE_GUI
#include "../ui/sdl/sdl_gui.h"
#endif
#include "stdlib.h"
#include "string.h"


#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"
#define PERFT "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main(int argc, char *argv[]) {

	Init_All();

	ChessBoard board;
    SearchInfo info;

    info.quit = BOOL_TYPE_FALSE;
    board.hashTable.init(64);
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    // Parse command line arguments
    int launchMode = 0; // 0=GUI (default), 1=UCI, 2=XBoard
    
    for(int ArgNum = 1; ArgNum < argc; ++ArgNum) {
    	if(strncmp(argv[ArgNum], "NoBook", 6) == 0) {
    		EngineOptions::instance().setBookEnabled(false);
    	} else if(strncmp(argv[ArgNum], "uci", 3) == 0) {
    		launchMode = 1;
    	} else if(strncmp(argv[ArgNum], "xboard", 6) == 0) {
    		launchMode = 2;
    	}
    }

	StaticEvaluator evaluator;
	UciProtocol     uciProtocol;
	XBoardProtocol  xboardProtocol;
	IProtocol*      protocol = nullptr;

	if (launchMode == 1) {
		protocol = &uciProtocol;
	} else if (launchMode == 2) {
		protocol = &xboardProtocol;
	}

	if (protocol) {
		protocol->run(board, info, evaluator);
	} else {
		// GUI mode (default)
#ifdef ENABLE_GUI
		GUI_Run(&board, &info, evaluator);
#else
		printf("Error: GUI mode not available in this build.\n");
		printf("Use command line argument 'uci' or 'xboard' for protocol modes.\n");
#endif
	}

	PolyBook_Clean();
	return 0;
}








