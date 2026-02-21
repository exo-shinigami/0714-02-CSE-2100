/**
 * @file board_representation.c
 * @brief Board representation and manipulation functions
 * 
 * Handles the chess board state including:
 * - Board initialization and reset
 * - FEN string parsing
 * - Board validation and consistency checking
 * - Material and piece list updates
 * - Board mirroring for evaluation testing
 * 
 * The board uses a hybrid representation:
 * - 120-square mailbox array for fast piece lookup
 * - Bitboards for efficient pawn operations
 * - Piece lists for fast iteration over pieces
 * 
 * @author Gambit Chess Team
 * @date February 2026
 */

#include "stdio.h"
#include "types_definitions.h"

int PceListOk(const ChessBoard *board) {
	int piece = PIECE_TYPE_WHITE_PAWN;
	int squareIndex;
	int number;
	for(piece = PIECE_TYPE_WHITE_PAWN; piece <= PIECE_TYPE_BLACK_KING; ++piece) {
		if(board->pieceCount[piece]<0 || board->pieceCount[piece]>=10) return BOOL_TYPE_FALSE;
	}

	if(board->pieceCount[PIECE_TYPE_WHITE_KING]!=1 || board->pieceCount[PIECE_TYPE_BLACK_KING]!=1) return BOOL_TYPE_FALSE;

	for(piece = PIECE_TYPE_WHITE_PAWN; piece <= PIECE_TYPE_BLACK_KING; ++piece) {
		for(number = 0; number < board->pieceCount[piece]; ++number) {
			squareIndex = board->pList[piece][number];
			if(!SqOnBoard(squareIndex)) return BOOL_TYPE_FALSE;
		}
	}
    return BOOL_TYPE_TRUE;
}

int Board_Check(const ChessBoard *board) {

	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_bigPce[2] = { 0, 0};
	int t_majPce[2] = { 0, 0};
	int t_minPce[2] = { 0, 0};
	int t_material[2] = { 0, 0};

	int sq64,t_piece,t_pce_num,sq120,colour,pcount;

	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};

	t_pawns[COLOR_TYPE_WHITE] = board->pawns[COLOR_TYPE_WHITE];
	t_pawns[COLOR_TYPE_BLACK] = board->pawns[COLOR_TYPE_BLACK];
	t_pawns[COLOR_TYPE_BOTH] = board->pawns[COLOR_TYPE_BOTH];

	// check piece lists
	for(t_piece = PIECE_TYPE_WHITE_PAWN; t_piece <= PIECE_TYPE_BLACK_KING; ++t_piece) {
		for(t_pce_num = 0; t_pce_num < board->pieceCount[t_piece]; ++t_pce_num) {
			sq120 = board->pList[t_piece][t_pce_num];
			ASSERT(board->pieces[sq120]==t_piece);
		}
	}

	// check piece count and other counters
	for(sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQUARE_64_TO_120(sq64);
		t_piece = board->pieces[sq120];
		t_pceNum[t_piece]++;
		colour = g_pieceCol[t_piece];
		if( PieceBig[t_piece] == BOOL_TYPE_TRUE) t_bigPce[colour]++;
		if( PieceMin[t_piece] == BOOL_TYPE_TRUE) t_minPce[colour]++;
		if( PieceMaj[t_piece] == BOOL_TYPE_TRUE) t_majPce[colour]++;

		t_material[colour] += g_pieceVal[t_piece];
	}

	for(t_piece = PIECE_TYPE_WHITE_PAWN; t_piece <= PIECE_TYPE_BLACK_KING; ++t_piece) {
		ASSERT(t_pceNum[t_piece]==board->pieceCount[t_piece]);
	}

	// check bitboards count
	pcount = BITBOARD_COUNT(t_pawns[COLOR_TYPE_WHITE]);
	ASSERT(pcount == board->pieceCount[PIECE_TYPE_WHITE_PAWN]);
	pcount = BITBOARD_COUNT(t_pawns[COLOR_TYPE_BLACK]);
	ASSERT(pcount == board->pieceCount[PIECE_TYPE_BLACK_PAWN]);
	pcount = BITBOARD_COUNT(t_pawns[COLOR_TYPE_BOTH]);
	ASSERT(pcount == (board->pieceCount[PIECE_TYPE_BLACK_PAWN] + board->pieceCount[PIECE_TYPE_WHITE_PAWN]));

	// check bitboards squares
	while(t_pawns[COLOR_TYPE_WHITE]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_WHITE]);
		ASSERT(board->pieces[SQUARE_64_TO_120(sq64)] == PIECE_TYPE_WHITE_PAWN);
	}

	while(t_pawns[COLOR_TYPE_BLACK]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_BLACK]);
		ASSERT(board->pieces[SQUARE_64_TO_120(sq64)] == PIECE_TYPE_BLACK_PAWN);
	}

	while(t_pawns[COLOR_TYPE_BOTH]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_BOTH]);
		ASSERT( (board->pieces[SQUARE_64_TO_120(sq64)] == PIECE_TYPE_BLACK_PAWN) || (board->pieces[SQUARE_64_TO_120(sq64)] == PIECE_TYPE_WHITE_PAWN) );
	}

	ASSERT(t_material[COLOR_TYPE_WHITE]==board->material[COLOR_TYPE_WHITE] && t_material[COLOR_TYPE_BLACK]==board->material[COLOR_TYPE_BLACK]);
	ASSERT(t_minPce[COLOR_TYPE_WHITE]==board->minPce[COLOR_TYPE_WHITE] && t_minPce[COLOR_TYPE_BLACK]==board->minPce[COLOR_TYPE_BLACK]);
	ASSERT(t_majPce[COLOR_TYPE_WHITE]==board->majPce[COLOR_TYPE_WHITE] && t_majPce[COLOR_TYPE_BLACK]==board->majPce[COLOR_TYPE_BLACK]);
	ASSERT(t_bigPce[COLOR_TYPE_WHITE]==board->bigPce[COLOR_TYPE_WHITE] && t_bigPce[COLOR_TYPE_BLACK]==board->bigPce[COLOR_TYPE_BLACK]);

	ASSERT(board->side==COLOR_TYPE_WHITE || board->side==COLOR_TYPE_BLACK);
	ASSERT(Board_GeneratePositionKey(board)==board->posKey);

	ASSERT(board->enPas==NO_SQ || ( g_ranksBoard[board->enPas]==RANK_TYPE_6 && board->side == COLOR_TYPE_WHITE)
		 || ( g_ranksBoard[board->enPas]==RANK_TYPE_3 && board->side == COLOR_TYPE_BLACK));

	ASSERT(board->pieces[board->KingSq[COLOR_TYPE_WHITE]] == PIECE_TYPE_WHITE_KING);
	ASSERT(board->pieces[board->KingSq[COLOR_TYPE_BLACK]] == PIECE_TYPE_BLACK_KING);

	ASSERT(board->castlePerm >= 0 && board->castlePerm <= 15);

	ASSERT(PceListOk(board));

	return BOOL_TYPE_TRUE;
}

void Board_UpdateListsMaterial(ChessBoard *board) {

	int piece,squareIndex,index,colour;

	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		squareIndex = index;
		piece = board->pieces[index];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece!=OFFBOARD && piece!= EMPTY) {
			colour = g_pieceCol[piece];
			ASSERT(SideValid(colour));

		    if( PieceBig[piece] == BOOL_TYPE_TRUE) board->bigPce[colour]++;
		    if( PieceMin[piece] == BOOL_TYPE_TRUE) board->minPce[colour]++;
		    if( PieceMaj[piece] == BOOL_TYPE_TRUE) board->majPce[colour]++;

			board->material[colour] += g_pieceVal[piece];

			ASSERT(board->pieceCount[piece] < 10 && board->pieceCount[piece] >= 0);

			board->pList[piece][board->pieceCount[piece]] = squareIndex;
			board->pieceCount[piece]++;


			if(piece==PIECE_TYPE_WHITE_KING) board->KingSq[COLOR_TYPE_WHITE] = squareIndex;
			if(piece==PIECE_TYPE_BLACK_KING) board->KingSq[COLOR_TYPE_BLACK] = squareIndex;

			if(piece==PIECE_TYPE_WHITE_PAWN) {
				BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_WHITE],SQUARE_120_TO_64(squareIndex));
				BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(squareIndex));
			} else if(piece==PIECE_TYPE_BLACK_PAWN) {
				BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_BLACK],SQUARE_120_TO_64(squareIndex));
				BITBOARD_SET_BIT(board->pawns[COLOR_TYPE_BOTH],SQUARE_120_TO_64(squareIndex));
			}
		}
	}
}

int Board_ParseFromFEN(char *fen, ChessBoard *board) {

	ASSERT(fen!=NULL);
	ASSERT(board!=NULL);

	int  rank = RANK_TYPE_8;
    int  file = FILE_TYPE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;
	int  sq120 = 0;

	Board_Reset(board);

	while ((rank >= RANK_TYPE_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = PIECE_TYPE_BLACK_PAWN; break;
            case 'r': piece = PIECE_TYPE_BLACK_ROOK; break;
            case 'n': piece = PIECE_TYPE_BLACK_KNIGHT; break;
            case 'b': piece = PIECE_TYPE_BLACK_BISHOP; break;
            case 'k': piece = PIECE_TYPE_BLACK_KING; break;
            case 'q': piece = PIECE_TYPE_BLACK_QUEEN; break;
            case 'P': piece = PIECE_TYPE_WHITE_PAWN; break;
            case 'R': piece = PIECE_TYPE_WHITE_ROOK; break;
            case 'N': piece = PIECE_TYPE_WHITE_KNIGHT; break;
            case 'B': piece = PIECE_TYPE_WHITE_BISHOP; break;
            case 'K': piece = PIECE_TYPE_WHITE_KING; break;
            case 'Q': piece = PIECE_TYPE_WHITE_QUEEN; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_TYPE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
			sq120 = SQUARE_64_TO_120(sq64);
            if (piece != EMPTY) {
                board->pieces[sq120] = piece;
            }
			file++;
        }
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	board->side = (*fen == 'w') ? COLOR_TYPE_WHITE : COLOR_TYPE_BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			case 'K': board->castlePerm |= CASTLE_TYPE_WKCA; break;
			case 'Q': board->castlePerm |= CASTLE_TYPE_WQCA; break;
			case 'k': board->castlePerm |= CASTLE_TYPE_BKCA; break;
			case 'q': board->castlePerm |= CASTLE_TYPE_BQCA; break;
			default:	     break;
        }
		fen++;
	}
	fen++;

	ASSERT(board->castlePerm>=0 && board->castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_TYPE_A && file <= FILE_TYPE_H);
		ASSERT(rank>=RANK_TYPE_1 && rank <= RANK_TYPE_8);

		board->enPas = FILE_RANK_TO_SQUARE(file,rank);
    }

	board->posKey = Board_GeneratePositionKey(board);

	Board_UpdateListsMaterial(board);

	return 0;
}

void Board_Reset(ChessBoard *board) {

	int index = 0;

	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		board->pieces[index] = OFFBOARD;
	}

	for(index = 0; index < 64; ++index) {
		board->pieces[SQUARE_64_TO_120(index)] = EMPTY;
	}

	for(index = 0; index < 2; ++index) {
		board->bigPce[index] = 0;
		board->majPce[index] = 0;
		board->minPce[index] = 0;
		board->material[index] = 0;
	}

	for(index = 0; index < 3; ++index) {
		board->pawns[index] = 0ULL;
	}

	for(index = 0; index < 13; ++index) {
		board->pieceCount[index] = 0;
	}

	board->KingSq[COLOR_TYPE_WHITE] = board->KingSq[COLOR_TYPE_BLACK] = NO_SQ;

	board->side = COLOR_TYPE_BOTH;
	board->enPas = NO_SQ;
	board->fiftyMove = 0;

	board->ply = 0;
	board->hisPly = 0;

	board->castlePerm = 0;

	board->posKey = 0ULL;
	
	// Initialize captured pieces tracking
	board->capturedWhiteCount = 0;
	board->capturedBlackCount = 0;
	for(index = 0; index < 16; ++index) {
		board->capturedWhite[index] = EMPTY;
		board->capturedBlack[index] = EMPTY;
	}

}
void Board_Print(const ChessBoard *board) {

	int squareIndex,file,rank,piece;

	printf("\nGame Board:\n\n");

	for(rank = RANK_TYPE_8; rank >= RANK_TYPE_1; rank--) {
		printf("%d  ",rank+1);
		for(file = FILE_TYPE_A; file <= FILE_TYPE_H; file++) {
			squareIndex = FILE_RANK_TO_SQUARE(file,rank);
			piece = board->pieces[squareIndex];
			printf("%3c",PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(file = FILE_TYPE_A; file <= FILE_TYPE_H; file++) {
		printf("%3c",'a'+file);
	}
	printf("\n");
	printf("side:%c\n",SideChar[board->side]);
	printf("enPas:%d\n",board->enPas);
	printf("castle:%c%c%c%c\n",
			board->castlePerm & CASTLE_TYPE_WKCA ? 'K' : '-',
			board->castlePerm & CASTLE_TYPE_WQCA ? 'Q' : '-',
			board->castlePerm & CASTLE_TYPE_BKCA ? 'k' : '-',
			board->castlePerm & CASTLE_TYPE_BQCA ? 'q' : '-'
			);
	printf("PosKey:%llX\n",board->posKey);
}

void Board_Mirror(ChessBoard *board) {

    int tempPiecesArray[64];
    int tempSide = board->side^1;
	int SwapPiece[13] = { EMPTY, PIECE_TYPE_BLACK_PAWN, PIECE_TYPE_BLACK_KNIGHT, PIECE_TYPE_BLACK_BISHOP, PIECE_TYPE_BLACK_ROOK, PIECE_TYPE_BLACK_QUEEN, PIECE_TYPE_BLACK_KING, PIECE_TYPE_WHITE_PAWN, PIECE_TYPE_WHITE_KNIGHT, PIECE_TYPE_WHITE_BISHOP, PIECE_TYPE_WHITE_ROOK, PIECE_TYPE_WHITE_QUEEN, PIECE_TYPE_WHITE_KING };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;

	int squareIndex;
	int tp;

    if (board->castlePerm & CASTLE_TYPE_WKCA) tempCastlePerm |= CASTLE_TYPE_BKCA;
    if (board->castlePerm & CASTLE_TYPE_WQCA) tempCastlePerm |= CASTLE_TYPE_BQCA;

    if (board->castlePerm & CASTLE_TYPE_BKCA) tempCastlePerm |= CASTLE_TYPE_WKCA;
    if (board->castlePerm & CASTLE_TYPE_BQCA) tempCastlePerm |= CASTLE_TYPE_WQCA;

	if (board->enPas != NO_SQ)  {
        tempEnPas = SQUARE_64_TO_120(Mirror64[SQUARE_120_TO_64(board->enPas)]);
    }

    for (squareIndex = 0; squareIndex < 64; squareIndex++) {
        tempPiecesArray[squareIndex] = board->pieces[SQUARE_64_TO_120(Mirror64[squareIndex])];
    }

    Board_Reset(board);

	for (squareIndex = 0; squareIndex < 64; squareIndex++) {
        tp = SwapPiece[tempPiecesArray[squareIndex]];
        board->pieces[SQUARE_64_TO_120(squareIndex)] = tp;
    }

	board->side = tempSide;
    board->castlePerm = tempCastlePerm;
    board->enPas = tempEnPas;

    board->posKey = Board_GeneratePositionKey(board);

	Board_UpdateListsMaterial(board);

    ASSERT(Board_Check(board));
}
