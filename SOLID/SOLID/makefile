# Gambit Chess Engine Makefile
# Refactored with proper folder structure
# February 2026

# Compiler and flags
CXX = g++
CXXFLAGS = -O2 -Wall -std=c++17 -Isrc/core/types -DENABLE_GUI
CXXFLAGS_GUI = -O2 -Wall -std=c++17 -Isrc/core/types -Isrc/ui/sdl -DENABLE_GUI
LDFLAGS_GUI = -static -mwindows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lfreetype -lharfbuzz -lglib-2.0 -lintl -lws2_32 -lole32 -lwinmm -lshlwapi -luuid -latomic -lpcre2-8 -lgraphite2 -lbrotlidec -lbrotlicommon -lbz2 -lpng16 -lz -lusp10 -lgdi32 -lrpcrt4 -luser32 -ldwrite -lm -lkernel32 -limm32 -loleaut32 -lversion -ladvapi32 -lsetupapi -lshell32 -ldinput8 -lpthread

# Directories
SRC_MAIN = src/main
SRC_CORE_TYPES = src/core/types
SRC_CORE_BOARD = src/core/board
SRC_CORE_MOVES = src/core/moves
SRC_CORE_BITBOARDS = src/core/bitboards
SRC_CORE_ATTACK = src/core/attack
SRC_ENGINE_SEARCH = src/engine/search
SRC_ENGINE_EVAL = src/engine/evaluation
SRC_ENGINE_HASH = src/engine/hashtable
SRC_UI_SDL = src/ui/sdl
SRC_UI_PROTOCOLS = src/ui/protocols
SRC_UTILS = src/utils
SRC_OPENINGBOOK = src/openingbook

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Source files (core engine only, no GUI)
SOURCES_CORE = \
	$(SRC_MAIN)/main_entry.cpp \
	$(SRC_CORE_TYPES)/types_data.cpp \
	$(SRC_CORE_BOARD)/board_representation.cpp \
	$(SRC_CORE_BOARD)/board_hashkeys.cpp \
	$(SRC_CORE_BOARD)/game_rules.cpp \
	$(SRC_CORE_BOARD)/board_validate.cpp \
	$(SRC_CORE_MOVES)/moves_generation.cpp \
	$(SRC_CORE_MOVES)/moves_execution.cpp \
	$(SRC_CORE_MOVES)/moves_io.cpp \
	$(SRC_CORE_BITBOARDS)/bitboards_utils.cpp \
	$(SRC_CORE_ATTACK)/attack_detection.cpp \
	$(SRC_ENGINE_SEARCH)/search_algorithm.cpp \
	$(SRC_ENGINE_SEARCH)/search_perft.cpp \
	$(SRC_ENGINE_EVAL)/evaluation_static.cpp \
	$(SRC_ENGINE_HASH)/hashtable_pv.cpp \
	$(SRC_UI_PROTOCOLS)/protocols_uci.cpp \
	$(SRC_UI_PROTOCOLS)/protocols_xboard.cpp \
	$(SRC_UTILS)/utils_misc.cpp \
	$(SRC_UTILS)/utils_init.cpp \
	$(SRC_OPENINGBOOK)/openingbook_poly.cpp \
	$(SRC_OPENINGBOOK)/openingbook_keys.cpp

# GUI source files (added to core for full build)
SOURCES_GUI = \
	$(SRC_UI_SDL)/sdl_gui.cpp \
	$(SRC_UI_SDL)/game_timer.cpp \
	$(SRC_UI_SDL)/move_history_tracker.cpp \
	$(SRC_UI_SDL)/gui_input_handler.cpp

# All source files
SOURCES = $(SOURCES_CORE) $(SOURCES_GUI)

# Object files
OBJECTS = $(SOURCES:%.cpp=$(OBJ_DIR)/%.o)

# Target executables
TARGET = $(BIN_DIR)/gambit.exe

# Default target - GUI version with SDL2 (statically linked)
all: directories $(TARGET)
	@cp $(TARGET) gambit.exe
	@echo "Build complete! Double-click gambit.exe to play"

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR)/$(SRC_MAIN)
	@mkdir -p $(OBJ_DIR)/$(SRC_CORE_TYPES)
	@mkdir -p $(OBJ_DIR)/$(SRC_CORE_BOARD)
	@mkdir -p $(OBJ_DIR)/$(SRC_CORE_MOVES)
	@mkdir -p $(OBJ_DIR)/$(SRC_CORE_BITBOARDS)
	@mkdir -p $(OBJ_DIR)/$(SRC_CORE_ATTACK)
	@mkdir -p $(OBJ_DIR)/$(SRC_ENGINE_SEARCH)
	@mkdir -p $(OBJ_DIR)/$(SRC_ENGINE_EVAL)
	@mkdir -p $(OBJ_DIR)/$(SRC_ENGINE_HASH)
	@mkdir -p $(OBJ_DIR)/$(SRC_UI_SDL)
	@mkdir -p $(OBJ_DIR)/$(SRC_UI_PROTOCOLS)
	@mkdir -p $(OBJ_DIR)/$(SRC_UTILS)
	@mkdir -p $(OBJ_DIR)/$(SRC_OPENINGBOOK)
	@mkdir -p $(BIN_DIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS_GUI)

# Compile core source files to object files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile GUI source files with SDL2 flags
$(OBJ_DIR)/$(SRC_UI_SDL)/%.o: $(SRC_UI_SDL)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_GUI) -c $< -o $@

# Clean build artifacts
clean:
	@rm -rf "$(BUILD_DIR)"
	@rm -f *.o
	@rm -f gambit.exe
	@echo "Clean complete"

# Clean and rebuild
rebuild: clean all

# Run the program
run: all
	gambit.exe

# Display help
help:
	@echo "Gambit Chess Engine - Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  make all      - Build GUI version with SDL2 (default)"
	@echo "  make clean    - Remove all build artifacts"
	@echo "  make rebuild  - Clean and build from scratch"
	@echo "  make run      - Build and run the game"
	@echo "  make help     - Display this help message"
	@echo ""
	@echo "Folder Structure:"
	@echo "  src/          - All source code"
	@echo "  build/obj/    - Object files"
	@echo "  build/bin/    - Executable output"
	@echo "  lib/          - External libraries (SDL2 DLLs)"
	@echo "  docs/         - Documentation"
	@echo "  scripts/      - Build and setup scripts"
	@echo ""
	@echo "Command Line Usage:"
	@echo "  gambit        - Launch GUI mode (default)"
	@echo "  gambit uci    - Launch UCI protocol mode"
	@echo "  gambit xboard - Launch XBoard protocol mode"

.PHONY: all directories clean rebuild run help
