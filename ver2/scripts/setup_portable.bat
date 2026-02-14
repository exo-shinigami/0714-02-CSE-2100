@echo off
REM Portable Gambit Setup Script for Windows

echo Setting up portable 32-bit Gambit chess engine...
echo.

REM Download 32-bit SDL2 DLLs
echo Downloading 32-bit SDL2 libraries...
echo Please download these files manually if this fails:
echo.
echo 1. SDL2 (32-bit MinGW): https://github.com/libsdl-org/SDL/releases/download/release-2.30.10/SDL2-devel-2.30.10-mingw.zip
echo    Extract: SDL2-2.30.10/i686-w64-mingw32/bin/SDL2.dll
echo.
echo 2. SDL2_ttf (32-bit MinGW): https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-mingw.zip
echo    Extract: SDL2_ttf-2.22.0/i686-w64-mingw32/bin/SDL2_ttf.dll
echo    And also: SDL2_ttf-2.22.0/i686-w64-mingw32/bin/libfreetype-6.dll
echo.
echo Copy the DLLs to the lib\ directory, then press any key to rebuild...
pause > nul

REM Rebuild executable using makefile
echo.
echo Rebuilding using makefile...
cd ..
make gui

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo.
    echo Your portable Gambit package is ready.
    echo Copy these files together to any Windows machine:
    echo   - build\bin\gambit.exe
    echo   - lib\SDL2.dll
    echo   - lib\SDL2_ttf.dll
    echo.
) else (
    echo.
    echo Build failed. Please check for errors above.
)

pause
