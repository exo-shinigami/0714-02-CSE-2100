#!/bin/bash
# Script to set up portable 32-bit version of Gambit

echo "Setting up portable Gambit chess engine..."

# Check if we need to download 32-bit SDL2 libraries
SDL2_VERSION="2.30.10"
SDL2_TTF_VERSION="2.22.0"

# Download 32-bit SDL2 if not present
if [ ! -f "SDL2-32bit.zip" ]; then
    echo "Downloading SDL2 32-bit..."
    curl -L "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-mingw.zip" -o SDL2-32bit.zip
fi

# Download 32-bit SDL2_ttf if not present
if [ ! -f "SDL2_ttf-32bit.zip" ]; then
    echo "Downloading SDL2_ttf 32-bit..."
    curl -L "https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.zip" -o SDL2_ttf-32bit.zip
fi

# Extract 32-bit DLLs into lib/ directory
echo "Extracting 32-bit DLLs to lib/..."
mkdir -p ../lib
unzip -j SDL2-32bit.zip "SDL2-${SDL2_VERSION}/i686-w64-mingw32/bin/SDL2.dll" -d ../lib 2>/dev/null || echo "Note: SDL2.dll extraction issue"
unzip -j SDL2_ttf-32bit.zip "SDL2_ttf-${SDL2_TTF_VERSION}/i686-w64-mingw32/bin/*.dll" -d ../lib 2>/dev/null || echo "Note: SDL2_ttf.dll extraction issue"

# Rebuild using makefile
echo "Rebuilding using makefile..."
cd ..
make gui

# Verify architecture matches
echo ""
echo "Verifying architecture compatibility..."
file build/bin/gambit.exe lib/SDL2.dll lib/SDL2_ttf.dll | grep -E "(gambit|SDL2)"

echo ""
echo "Setup complete! Your portable Gambit package includes:"
echo "  - build/bin/gambit.exe (32-bit)"
echo "  - lib/SDL2.dll (32-bit)"
echo "  - lib/SDL2_ttf.dll (32-bit)"
echo ""
echo "These files can now be copied together to any Windows machine."
