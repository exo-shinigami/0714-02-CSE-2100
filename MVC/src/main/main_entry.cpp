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
#include <memory>
#include "types_definitions.h"
#include "../mvc/adapters/CoreModelAdapter.h"
#ifdef ENABLE_GUI
#include "../mvc/controllers/ControllerImpl.h"
#include "../mvc/views/SDLView.h"
#include "../ui/sdl/services/engine_move_policy.h"
#endif
#include "stdlib.h"
#include "string.h"


#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"
#define pERFT "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

enum class LaunchMode {
	GUI,
	UCI,
	XBOARD
};

static LaunchMode parseLaunchMode(int argc, char *argv[]) {
	LaunchMode mode = LaunchMode::GUI;
	for(int argNum = 1; argNum < argc; ++argNum) {
		if(strncmp(argv[argNum], "NoBook", 6) == 0) {
			EngineOptions::instance().setBookEnabled(false);
		} else if(strncmp(argv[argNum], "uci", 3) == 0) {
			mode = LaunchMode::UCI;
		} else if(strncmp(argv[argNum], "xboard", 6) == 0) {
			mode = LaunchMode::XBOARD;
		}
	}
	return mode;
}

static IProtocol* resolveProtocol(LaunchMode mode, UciProtocol& uciProtocol, XBoardProtocol& xboardProtocol) {
	switch (mode) {
		case LaunchMode::UCI: return &uciProtocol;
		case LaunchMode::XBOARD: return &xboardProtocol;
		default: return nullptr;
	}
}

int main(int argc, char *argv[]) {

	initAll();

	CoreModelAdapter model;
	if (!model.initialize()) {
		printf("Failed to initialize model.\n");
		return 1;
	}

    SearchInfo info;

    info.quit = BOOL_TYPE_FALSE;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    const LaunchMode launchMode = parseLaunchMode(argc, argv);

	StaticEvaluator evaluator;
	UciProtocol     uciProtocol;
	XBoardProtocol  xboardProtocol;
	IProtocol*      protocol = nullptr;

	protocol = resolveProtocol(launchMode, uciProtocol, xboardProtocol);

	if (protocol) {
		protocol->run(model, info, evaluator);
	} else {
		// GUI mode (default)
#ifdef ENABLE_GUI
		auto view = std::make_unique<SDLView>();
		const IEngineMovePolicy& movePolicy = defaultEngineMovePolicy();
		ControllerImpl controller(&model, view.get(), &info, evaluator, movePolicy);
		controller.start();
#else
		printf("Error: GUI mode not available in this build.\n");
		printf("Use command line argument 'uci' or 'xboard' for protocol modes.\n");
#endif
	}

	polyBookClean();
	return 0;
}








