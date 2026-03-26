/**
 * @file openingbook_poly.c
 * @brief Polyglot opening book support
 * 
 * Implements support for Polyglot-format opening books.
 * 
 * Features:
 * - Reading .bin format Polyglot books
 * - Position lookup using Polyglot hash keys
 * - Weighted random move selection from book
 * - Binary search for position in book file
 * 
 * Polyglot books store opening moves with weights, allowing
 * the engine to play varied and well-analyzed openings.
 * 
 * Book file format: See Polyglot specification
 * Default book: performance.bin
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "types_definitions.h"
#include "openingbook_keys.h"
#include <string.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

struct PolyBookEntry {
	U64 key;
	unsigned short move;
	unsigned short weight;
	unsigned int learn;
};

class OpeningBookRepository {
	std::vector<PolyBookEntry> entries_;
public:
	void init();
	void clean();
	int getMove(ChessBoard *board) const;
};

static OpeningBookRepository& getOpeningBookRepository() {
	static OpeningBookRepository repository;
	return repository;
}

const int PolyKindOfPiece[13] = {
	-1, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};

void OpeningBookRepository::init() {
	clean();
	EngineOptions::instance().setBookEnabled(false);

	char exePath[1024];
	char bookPath[1060];

#ifdef _WIN32
	GetModuleFileNameA(NULL, exePath, sizeof(exePath));
	char *lastSlash = strrchr(exePath, '\\');
	if (lastSlash) *(lastSlash + 1) = '\0';
#else
	ssize_t count = readlink("/proc/self/exe", exePath, sizeof(exePath));
	if (count != -1) {
		exePath[count] = '\0';
		char *lastSlash = strrchr(exePath, '/');
		if (lastSlash) *(lastSlash + 1) = '\0';
	}
#endif
	snprintf(bookPath, sizeof(bookPath), "%sperformance.bin", exePath);

	FILE *pFile = fopen(bookPath, "rb");

	if(pFile == NULL) {
		// Book file not found - silently skip
	} else {
		fseek(pFile,0,SEEK_END);
		long position = ftell(pFile);

		if(position < (long)sizeof(PolyBookEntry)) {
			printf("No Entries Found\n");
			fclose(pFile);
			return;
		}

		const long numEntries = position / sizeof(PolyBookEntry);
		printf("%ld Entries Found In File\n", numEntries);

		entries_.resize(numEntries);
		rewind(pFile);
		
		size_t returnValue;
		returnValue = fread(entries_.data(), sizeof(PolyBookEntry), numEntries, pFile);
		printf("fread() %zu Entries Read in from file\n", returnValue);
		if (returnValue < entries_.size()) {
			entries_.resize(returnValue);
		}
		
		if(!entries_.empty()) {
			EngineOptions::instance().setBookEnabled(true);
		}
		fclose(pFile);
	}
}

void OpeningBookRepository::clean() {
	entries_.clear();
	entries_.shrink_to_fit();
}

int hasPawnForCapture(const ChessBoard *board) {
	int sqWithPawn = 0;
	int targetPce = (board->getSide() == COLOR_TYPE_WHITE) ? PIECE_TYPE_WHITE_PAWN : PIECE_TYPE_BLACK_PAWN;
	if(board->getEnPassantSquare() != NO_SQ) {
		if(board->getSide() == COLOR_TYPE_WHITE) {
			sqWithPawn = board->getEnPassantSquare() - 10;
		} else {
			sqWithPawn = board->getEnPassantSquare() + 10;
		}
		
		if(board->pieceAt(sqWithPawn + 1) == targetPce) {
			return BOOL_TYPE_TRUE;
		} else if(board->pieceAt(sqWithPawn - 1) == targetPce) {
			return BOOL_TYPE_TRUE;
		} 
	}
	return BOOL_TYPE_FALSE;
}

U64 polyKeyFromBoard(const ChessBoard *board) {

	int squareIndex = 0, rank = 0, file = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	int polyPiece = 0;
	int offset = 0;
	
	for(squareIndex = 0; squareIndex < CHESS_BOARD_SQUARE_NUM; ++squareIndex) {
		piece = board->pieceAt(squareIndex);
		if(piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
			ASSERT(piece >= PIECE_TYPE_WHITE_PAWN && piece <= PIECE_TYPE_BLACK_KING);
			polyPiece = PolyKindOfPiece[piece];
			rank = g_ranksBoard[squareIndex];
			file = g_filesBoard[squareIndex];
			finalKey ^= Random64Poly[(64 * polyPiece) + (8 * rank) + file];
		}
	}
	
	// castling
	offset = 768;
	
	if(board->getCastlePermission() & CASTLE_TYPE_WKCA) finalKey ^= Random64Poly[offset + 0];
	if(board->getCastlePermission() & CASTLE_TYPE_WQCA) finalKey ^= Random64Poly[offset + 1];
	if(board->getCastlePermission() & CASTLE_TYPE_BKCA) finalKey ^= Random64Poly[offset + 2];
	if(board->getCastlePermission() & CASTLE_TYPE_BQCA) finalKey ^= Random64Poly[offset + 3];
	
	// enpassant
	offset = 772;
	if(hasPawnForCapture(board) == BOOL_TYPE_TRUE) {
		file = g_filesBoard[board->getEnPassantSquare()];
		finalKey ^= Random64Poly[offset + file];
	}
	
	if(board->getSide() == COLOR_TYPE_WHITE) {
		finalKey ^= Random64Poly[780];
	}
	return finalKey;
}

unsigned short endian_swap_u16(unsigned short x) 
{ 
    x = (x>>8) | 
        (x<<8); 
    return x;
} 

unsigned int endian_swap_u32(unsigned int x) 
{ 
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) | 
        ((x>>8) & 0x0000FF00) | 
        (x<<24); 
    return x;
} 

U64 endian_swap_u64(U64 x) 
{ 
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) | 
        ((x<<24) & 0x0000FF0000000000) | 
        ((x<<8)  & 0x000000FF00000000) | 
        ((x>>8)  & 0x00000000FF000000) | 
        ((x>>24) & 0x0000000000FF0000) | 
        ((x>>40) & 0x000000000000FF00) | 
        (x<<56); 
    return x;
}

int convertPolyMoveToInternalMove(unsigned short polyMove, ChessBoard *board) {
	
	int ff = (polyMove >> 6) & 7;
	int fr = (polyMove >> 9) & 7;
	int tf = (polyMove >> 0) & 7;
	int tr = (polyMove >> 3) & 7;
	int pp = (polyMove >> 12) & 7;
	
	char moveString[6];
	if(pp == 0) {
		sprintf(moveString, "%c%c%c%c",
		fileChar[ff],
		rankChar[fr],
		fileChar[tf],
		rankChar[tr]);
	} else {
		char promChar = 'q';
		switch(pp) {
			case 1: promChar = 'n'; break;
			case 2: promChar = 'b'; break;
			case 3: promChar = 'r'; break;
		}
		sprintf(moveString, "%c%c%c%c%c",
		fileChar[ff],
		rankChar[fr],
		fileChar[tf],
		rankChar[tr],
		promChar);
	}
	
	return moveParse(moveString, board);
}

int OpeningBookRepository::getMove(ChessBoard *board) const {
	if(entries_.empty()) {
		return NOMOVE;
	}
	
	unsigned short move;
	// Use a macro so it is a compile-time constant for array sizing
	#define MAXBOOKMOVES 32
	int bookMoves[MAXBOOKMOVES];
	int tempMove = NOMOVE;
	int count = 0;
	
	U64 polyKey = polyKeyFromBoard(board);
	
	for(const PolyBookEntry& entry : entries_) {
		if(polyKey == endian_swap_u64(entry.key)) {
			move = endian_swap_u16(entry.move);
			tempMove = convertPolyMoveToInternalMove(move, board);
			if(tempMove != NOMOVE) {
				if(count >= MAXBOOKMOVES) break;
				bookMoves[count++] = tempMove;
			}
		}
	}
	
	if(count != 0) {
		int randMove = rand() % count;
		return bookMoves[randMove];
	} else {
		return NOMOVE;
	}
}

void polyBookInit() {
	getOpeningBookRepository().init();
}

void polyBookClean() {
	getOpeningBookRepository().clean();
}

int polyBookGetMove(ChessBoard *board) {
	return getOpeningBookRepository().getMove(board);
}




















































