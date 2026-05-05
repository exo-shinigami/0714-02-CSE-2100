/**
 * @file main_entry.c
 * @brief Main entry point for Gambit Chess Engine
 * 
 * This file contains the main() function and handles:
 * - Engine initialization
 * - Command-line argument parsing
 * - GUI mode launch (default)
 * - Program startup and shutdown
 * 
 * Usage:
 *   gambit           - Launch GUI mode (default)
 *   gambit NoBook    - Launch GUI with opening book disabled
 * 
 * @author Gambit Chess Team
 * @date February 2026
 * @version 2.0
 */

#include "stdio.h"
#include "types_definitions.h"
#include "../mvc/adapters/CoreModelAdapter.h"
#ifdef ENABLE_GUI
#include "../mvc/controllers/ControllerImpl.h"
#include "../mvc/views/SDLView.h"
#include "../ui/sdl/services/engine_move_policy.h"
#endif
#include "stdlib.h"
#include "string.h"

int main(int argc, char *argv[]) {

	initAll();

	CoreModelAdapter model;
	if (!model.initialize()) {
		printf("Failed to initialize model.\n");
		return 1;
	}

    SearchInfo info;

    info.quit = BOOL_TYPE_FALSE;
	for (int argNum = 1; argNum < argc; ++argNum) {
		if (strncmp(argv[argNum], "NoBook", 6) == 0) {
			EngineOptions::instance().setBookEnabled(false);
		}
	}

#ifdef ENABLE_GUI
	StaticEvaluator evaluator;
	SDLView view;
	const IEngineMovePolicy& movePolicy = defaultEngineMovePolicy();
	ControllerImpl controller(&model, &view, &info, evaluator, movePolicy);
	controller.start();
#else
	printf("Error: GUI mode not available in this build.\n");
#endif

	polyBookClean();
	return 0;
}








