all:
	gcc xboard.c gambit.c uci.c evaluate.c pvtable.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c movegen.c validate.c makemove.c perft.c search.c misc.c polybook.c polykeys.c gui.c -o gambit -O2 -mwindows -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
	@echo "Copying required DLL files..."
	@cp -f ../../SDL2-2.30.10/i686-w64-mingw32/bin/SDL2.dll . 2>/dev/null || echo "SDL2.dll not found in expected location"
	@cp -f ../../SDL2_ttf-2.22.0/i686-w64-mingw32/bin/SDL2_ttf.dll . 2>/dev/null || echo "SDL2_ttf.dll not found in expected location"
	@echo "Build complete! gambit.exe and required DLLs are ready."