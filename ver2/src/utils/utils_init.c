/**
 * @file utils_init.c
 * @brief Initialization of global arrays and data structures
 * 
 * This module initializes all the lookup tables, masks, and hash keys
 * required by the chess engine before it can start processing positions.
 * 
 * Key responsibilities:
 * - Square conversion tables (120-square to 64-square)
 * - Bitboard masks for set/clear operations
 * - Zobrist hash keys for position hashing
 * - Evaluation masks (passed pawns, isolated pawns)
 * - File and rank lookup tables
 * - MVV-LVA scoring tables
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "types_definitions.h"
#include "stdio.h"
#include "stdlib.h"

#define RAND_64 	((U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60 )

int g_square120To64[CHESS_BOARD_SQUARE_NUM];
int g_square64To120[64];

U64 g_bitSetMask[64];
U64 g_bitClearMask[64];

U64 g_pieceKeys[13][120];
U64 g_sideKey;
U64 g_castleKeys[16];

int g_filesBoard[CHESS_BOARD_SQUARE_NUM];
int g_ranksBoard[CHESS_BOARD_SQUARE_NUM];

U64 g_fileBBMask[8];
U64 g_rankBBMask[8];

U64 g_blackPassedMask[64];
U64 g_whitePassedMask[64];
U64 g_isolatedMask[64];

S_OPTIONS EngineOptions[1];

void Init_EvalMasks() {

	int squareIndex, tsq, r, f;

	for(squareIndex = 0; squareIndex < 8; ++squareIndex) {
        g_fileBBMask[squareIndex] = 0ULL;
		g_rankBBMask[squareIndex] = 0ULL;
	}

	for(r = RANK_TYPE_8; r >= RANK_TYPE_1; r--) {
        for (f = FILE_TYPE_A; f <= FILE_TYPE_H; f++) {
            squareIndex = r * 8 + f;
            g_fileBBMask[f] |= (1ULL << squareIndex);
            g_rankBBMask[r] |= (1ULL << squareIndex);
        }
	}

	for(squareIndex = 0; squareIndex < 64; ++squareIndex) {
		g_isolatedMask[squareIndex] = 0ULL;
		g_whitePassedMask[squareIndex] = 0ULL;
		g_blackPassedMask[squareIndex] = 0ULL;
    }

	for(squareIndex = 0; squareIndex < 64; ++squareIndex) {
		tsq = squareIndex + 8;

        while(tsq < 64) {
            g_whitePassedMask[squareIndex] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = squareIndex - 8;
        while(tsq >= 0) {
            g_blackPassedMask[squareIndex] |= (1ULL << tsq);
            tsq -= 8;
        }

        if(g_filesBoard[SQUARE_64_TO_120(squareIndex)] > FILE_TYPE_A) {
            g_isolatedMask[squareIndex] |= g_fileBBMask[g_filesBoard[SQUARE_64_TO_120(squareIndex)] - 1];

            tsq = squareIndex + 7;
            while(tsq < 64) {
                g_whitePassedMask[squareIndex] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = squareIndex - 9;
            while(tsq >= 0) {
                g_blackPassedMask[squareIndex] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

        if(g_filesBoard[SQUARE_64_TO_120(squareIndex)] < FILE_TYPE_H) {
            g_isolatedMask[squareIndex] |= g_fileBBMask[g_filesBoard[SQUARE_64_TO_120(squareIndex)] + 1];

            tsq = squareIndex + 9;
            while(tsq < 64) {
                g_whitePassedMask[squareIndex] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = squareIndex - 7;
            while(tsq >= 0) {
                g_blackPassedMask[squareIndex] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
	}
}

void Init_FilesRanksBoard() {

	int index = 0;
	int file = FILE_TYPE_A;
	int rank = RANK_TYPE_1;
	int squareIndex = A1;

	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		g_filesBoard[index] = OFFBOARD;
		g_ranksBoard[index] = OFFBOARD;
	}

	for(rank = RANK_TYPE_1; rank <= RANK_TYPE_8; ++rank) {
		for(file = FILE_TYPE_A; file <= FILE_TYPE_H; ++file) {
			squareIndex = FILE_RANK_TO_SQUARE(file,rank);
			g_filesBoard[squareIndex] = file;
			g_ranksBoard[squareIndex] = rank;
		}
	}
}

void Init_HashKeys() {

	int index = 0;
	int index2 = 0;
	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < 120; ++index2) {
			g_pieceKeys[index][index2] = RAND_64;
		}
	}
	g_sideKey = RAND_64;
	for(index = 0; index < 16; ++index) {
		g_castleKeys[index] = RAND_64;
	}

}

void Init_BitMasks() {
	int index = 0;

	for(index = 0; index < 64; index++) {
		g_bitSetMask[index] = 0ULL;
		g_bitClearMask[index] = 0ULL;
	}

	for(index = 0; index < 64; index++) {
		g_bitSetMask[index] |= (1ULL << index);
		g_bitClearMask[index] = ~g_bitSetMask[index];
	}
}

void Init_Square120To64() {

	int index = 0;
	int file = FILE_TYPE_A;
	int rank = RANK_TYPE_1;
	int squareIndex = A1;
	int sq64 = 0;
	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		g_square120To64[index] = 65;
	}

	for(index = 0; index < 64; ++index) {
		g_square64To120[index] = 120;
	}

	for(rank = RANK_TYPE_1; rank <= RANK_TYPE_8; ++rank) {
		for(file = FILE_TYPE_A; file <= FILE_TYPE_H; ++file) {
			squareIndex = FILE_RANK_TO_SQUARE(file,rank);
			ASSERT(SqOnBoard(squareIndex));
			g_square64To120[sq64] = squareIndex;
			g_square120To64[squareIndex] = sq64;
			sq64++;
		}
	}
}

void Init_All() {
	Init_Square120To64();
	Init_BitMasks();
	Init_HashKeys();
	Init_FilesRanksBoard();
	Init_EvalMasks();
	Init_MvvLva();
	PolyBook_Init();
}
