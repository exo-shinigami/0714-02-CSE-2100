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

int pceListOk(const ChessBoard *board) {
	int piece = PIECE_TYPE_WHITE_PAWN;
	int squareIndex;
	int number;
	for(piece = PIECE_TYPE_WHITE_PAWN; piece <= PIECE_TYPE_BLACK_KING; ++piece) {
		if(board->pieceCountAt(piece)<0 || board->pieceCountAt(piece)>=10) return BOOL_TYPE_FALSE;
	}

	if(board->pieceCountAt(PIECE_TYPE_WHITE_KING)!=1 || board->pieceCountAt(PIECE_TYPE_BLACK_KING)!=1) return BOOL_TYPE_FALSE;

	for(piece = PIECE_TYPE_WHITE_PAWN; piece <= PIECE_TYPE_BLACK_KING; ++piece) {
		for(number = 0; number < board->pieceCountAt(piece); ++number) {
			squareIndex = board->pieceListAt(piece, number);
			if(!sqOnBoard(squareIndex)) return BOOL_TYPE_FALSE;
		}
	}
    return BOOL_TYPE_TRUE;
}

int ChessBoard::check() const {
	const ChessBoard *board = this;

	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_bigPce[2] = { 0, 0};
	int t_majPce[2] = { 0, 0};
	int t_minPce[2] = { 0, 0};
	int t_material[2] = { 0, 0};

	int sq64,t_piece,t_pce_num,sq120,colour;

	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};

	t_pawns[COLOR_TYPE_WHITE] = board->pawnsAt(COLOR_TYPE_WHITE);
	t_pawns[COLOR_TYPE_BLACK] = board->pawnsAt(COLOR_TYPE_BLACK);
	t_pawns[COLOR_TYPE_BOTH] = board->pawnsAt(COLOR_TYPE_BOTH);

	// check piece lists
	for(t_piece = PIECE_TYPE_WHITE_PAWN; t_piece <= PIECE_TYPE_BLACK_KING; ++t_piece) {
		for(t_pce_num = 0; t_pce_num < board->pieceCountAt(t_piece); ++t_pce_num) {
			sq120 = board->pieceListAt(t_piece, t_pce_num);
			ASSERT(board->pieceAt(sq120)==t_piece);
		}
	}

	// check piece count and other counters
	for(sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQUARE_64_TO_120(sq64);
		t_piece = board->pieceAt(sq120);
		t_pceNum[t_piece]++;
		colour = g_pieceCol[t_piece];
		if( pieceBig[t_piece] == BOOL_TYPE_TRUE) t_bigPce[colour]++;
		if( pieceMin[t_piece] == BOOL_TYPE_TRUE) t_minPce[colour]++;
		if( pieceMaj[t_piece] == BOOL_TYPE_TRUE) t_majPce[colour]++;

		t_material[colour] += g_pieceVal[t_piece];
	}

	for(t_piece = PIECE_TYPE_WHITE_PAWN; t_piece <= PIECE_TYPE_BLACK_KING; ++t_piece) {
		ASSERT(t_pceNum[t_piece]==board->pieceCountAt(t_piece));
	}

	// check bitboards count
	ASSERT(BITBOARD_COUNT(t_pawns[COLOR_TYPE_WHITE]) == board->pieceCountAt(PIECE_TYPE_WHITE_PAWN));
	ASSERT(BITBOARD_COUNT(t_pawns[COLOR_TYPE_BLACK]) == board->pieceCountAt(PIECE_TYPE_BLACK_PAWN));
	ASSERT(BITBOARD_COUNT(t_pawns[COLOR_TYPE_BOTH]) == (board->pieceCountAt(PIECE_TYPE_BLACK_PAWN) + board->pieceCountAt(PIECE_TYPE_WHITE_PAWN)));

	// check bitboards squares
	while(t_pawns[COLOR_TYPE_WHITE]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_WHITE]);
		ASSERT(board->pieceAt(SQUARE_64_TO_120(sq64)) == PIECE_TYPE_WHITE_PAWN);
	}

	while(t_pawns[COLOR_TYPE_BLACK]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_BLACK]);
		ASSERT(board->pieceAt(SQUARE_64_TO_120(sq64)) == PIECE_TYPE_BLACK_PAWN);
	}

	while(t_pawns[COLOR_TYPE_BOTH]) {
		sq64 = BITBOARD_POP(&t_pawns[COLOR_TYPE_BOTH]);
		ASSERT( (board->pieceAt(SQUARE_64_TO_120(sq64)) == PIECE_TYPE_BLACK_PAWN) || (board->pieceAt(SQUARE_64_TO_120(sq64)) == PIECE_TYPE_WHITE_PAWN) );
	}

	ASSERT(t_material[COLOR_TYPE_WHITE]==board->materialAt(COLOR_TYPE_WHITE) && t_material[COLOR_TYPE_BLACK]==board->materialAt(COLOR_TYPE_BLACK));
	ASSERT(t_minPce[COLOR_TYPE_WHITE]==board->minorPieceCountAt(COLOR_TYPE_WHITE) && t_minPce[COLOR_TYPE_BLACK]==board->minorPieceCountAt(COLOR_TYPE_BLACK));
	ASSERT(t_majPce[COLOR_TYPE_WHITE]==board->majorPieceCountAt(COLOR_TYPE_WHITE) && t_majPce[COLOR_TYPE_BLACK]==board->majorPieceCountAt(COLOR_TYPE_BLACK));
	ASSERT(t_bigPce[COLOR_TYPE_WHITE]==board->bigPieceCountAt(COLOR_TYPE_WHITE) && t_bigPce[COLOR_TYPE_BLACK]==board->bigPieceCountAt(COLOR_TYPE_BLACK));

	ASSERT(board->getSide()==COLOR_TYPE_WHITE || board->getSide()==COLOR_TYPE_BLACK);
	ASSERT(board->generatePositionKey()==board->getPositionKey());

	ASSERT(board->getEnPassantSquare()==NO_SQ || ( g_ranksBoard[board->getEnPassantSquare()]==RANK_TYPE_6 && board->getSide() == COLOR_TYPE_WHITE)
		 || ( g_ranksBoard[board->getEnPassantSquare()]==RANK_TYPE_3 && board->getSide() == COLOR_TYPE_BLACK));

	ASSERT(board->pieceAt(board->getKingSquare(COLOR_TYPE_WHITE)) == PIECE_TYPE_WHITE_KING);
	ASSERT(board->pieceAt(board->getKingSquare(COLOR_TYPE_BLACK)) == PIECE_TYPE_BLACK_KING);

	ASSERT(board->getCastlePermission() >= 0 && board->getCastlePermission() <= 15);

	ASSERT(pceListOk(board));

	return BOOL_TYPE_TRUE;
}

void ChessBoard::updateListsMaterial() {
	ChessBoard *board = this;

	int piece,squareIndex,index,colour;

	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		squareIndex = index;
		piece = board->pieceAt(index);
		ASSERT(pceValidEmptyOffbrd(piece));
		if(piece!=OFFBOARD && piece!= EMPTY) {
			colour = g_pieceCol[piece];
			ASSERT(sideValid(colour));

		    if( pieceBig[piece] == BOOL_TYPE_TRUE) board->bigPieceCountAt(colour)++;
		    if( pieceMin[piece] == BOOL_TYPE_TRUE) board->minorPieceCountAt(colour)++;
		    if( pieceMaj[piece] == BOOL_TYPE_TRUE) board->majorPieceCountAt(colour)++;

			board->materialAt(colour) += g_pieceVal[piece];

			ASSERT(board->pieceCountAt(piece) < 10 && board->pieceCountAt(piece) >= 0);

			board->pieceListAt(piece, board->pieceCountAt(piece)) = squareIndex;
			board->pieceCountAt(piece)++;


			if(piece==PIECE_TYPE_WHITE_KING) board->setKingSquare(COLOR_TYPE_WHITE, squareIndex);
			if(piece==PIECE_TYPE_BLACK_KING) board->setKingSquare(COLOR_TYPE_BLACK, squareIndex);

			if(piece==PIECE_TYPE_WHITE_PAWN) {
				BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_WHITE),SQUARE_120_TO_64(squareIndex));
				BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(squareIndex));
			} else if(piece==PIECE_TYPE_BLACK_PAWN) {
				BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_BLACK),SQUARE_120_TO_64(squareIndex));
				BITBOARD_SET_BIT(board->pawnsAt(COLOR_TYPE_BOTH),SQUARE_120_TO_64(squareIndex));
			}
		}
	}
}

int ChessBoard::parseFromFEN(const char *fen) {
	ChessBoard *board = this;

	ASSERT(fen!=NULL);
	ASSERT(board!=NULL);

	int  rank = RANK_TYPE_8;
    int  file = FILE_TYPE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;
	int  sq120 = 0;

	board->reset();

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
                board->pieceAt(sq120) = piece;
            }
			file++;
        }
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	board->setSide((*fen == 'w') ? COLOR_TYPE_WHITE : COLOR_TYPE_BLACK);
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
			switch(*fen) {
				case 'K': board->orCastlePermission(CASTLE_TYPE_WKCA); break;
				case 'Q': board->orCastlePermission(CASTLE_TYPE_WQCA); break;
				case 'k': board->orCastlePermission(CASTLE_TYPE_BKCA); break;
				case 'q': board->orCastlePermission(CASTLE_TYPE_BQCA); break;
			default:	     break;
        }
		fen++;
	}
	fen++;

	ASSERT(board->getCastlePermission()>=0 && board->getCastlePermission() <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_TYPE_A && file <= FILE_TYPE_H);
		ASSERT(rank>=RANK_TYPE_1 && rank <= RANK_TYPE_8);

		board->setEnPassantSquare(fILERANKTOSQUARE(file,rank));
    }

	board->setPositionKey(board->generatePositionKey());

	board->updateListsMaterial();

	return 0;
}

void ChessBoard::reset() {
	ChessBoard *board = this;

	int index = 0;

	for(index = 0; index < CHESS_BOARD_SQUARE_NUM; ++index) {
		board->pieceAt(index) = OFFBOARD;
	}

	for(index = 0; index < 64; ++index) {
		board->pieceAt(SQUARE_64_TO_120(index)) = EMPTY;
	}

	for(index = 0; index < 2; ++index) {
		board->bigPieceCountAt(index) = 0;
		board->majorPieceCountAt(index) = 0;
		board->minorPieceCountAt(index) = 0;
		board->materialAt(index) = 0;
	}

	for(index = 0; index < 3; ++index) {
		board->pawnsAt(index) = 0ULL;
	}

	for(index = 0; index < 13; ++index) {
		board->pieceCountAt(index) = 0;
	}

	board->setKingSquare(COLOR_TYPE_WHITE, NO_SQ);
	board->setKingSquare(COLOR_TYPE_BLACK, NO_SQ);

	board->setSide(COLOR_TYPE_BOTH);
	board->setEnPassantSquare(NO_SQ);
	board->setFiftyMoveCounter(0);

	board->setPly(0);
	board->setHistoryPly(0);

	board->setCastlePermission(0);

	board->setPositionKey(0ULL);

}
ChessBoard::ChessBoard() {
	reset();
}

void ChessBoard::print() const {
	const ChessBoard *board = this;

	int squareIndex,file,rank,piece;

	printf("\nGame Board:\n\n");

	for(rank = RANK_TYPE_8; rank >= RANK_TYPE_1; rank--) {
		printf("%d  ",rank+1);
		for(file = FILE_TYPE_A; file <= FILE_TYPE_H; file++) {
			squareIndex = fILERANKTOSQUARE(file,rank);
			piece = board->pieceAt(squareIndex);
			printf("%3c",pceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(file = FILE_TYPE_A; file <= FILE_TYPE_H; file++) {
		printf("%3c",'a'+file);
	}
	printf("\n");
	printf("side:%c\n",sideChar[board->getSide()]);
	printf("enPas:%d\n",board->getEnPassantSquare());
	printf("castle:%c%c%c%c\n",
			board->getCastlePermission() & CASTLE_TYPE_WKCA ? 'K' : '-',
			board->getCastlePermission() & CASTLE_TYPE_WQCA ? 'Q' : '-',
			board->getCastlePermission() & CASTLE_TYPE_BKCA ? 'k' : '-',
			board->getCastlePermission() & CASTLE_TYPE_BQCA ? 'q' : '-'
			);
	printf("PosKey:%llX\n",board->getPositionKey());
}

void ChessBoard::mirror() {
	ChessBoard *board = this;

    int tempPiecesArray[64];
	int tempSide = board->getSide()^1;
	int swapPiece[13] = { EMPTY, PIECE_TYPE_BLACK_PAWN, PIECE_TYPE_BLACK_KNIGHT, PIECE_TYPE_BLACK_BISHOP, PIECE_TYPE_BLACK_ROOK, PIECE_TYPE_BLACK_QUEEN, PIECE_TYPE_BLACK_KING, PIECE_TYPE_WHITE_PAWN, PIECE_TYPE_WHITE_KNIGHT, PIECE_TYPE_WHITE_BISHOP, PIECE_TYPE_WHITE_ROOK, PIECE_TYPE_WHITE_QUEEN, PIECE_TYPE_WHITE_KING };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;

	int squareIndex;
	int tp;

	if (board->getCastlePermission() & CASTLE_TYPE_WKCA) tempCastlePerm |= CASTLE_TYPE_BKCA;
	if (board->getCastlePermission() & CASTLE_TYPE_WQCA) tempCastlePerm |= CASTLE_TYPE_BQCA;

	if (board->getCastlePermission() & CASTLE_TYPE_BKCA) tempCastlePerm |= CASTLE_TYPE_WKCA;
	if (board->getCastlePermission() & CASTLE_TYPE_BQCA) tempCastlePerm |= CASTLE_TYPE_WQCA;

	if (board->getEnPassantSquare() != NO_SQ)  {
		tempEnPas = SQUARE_64_TO_120(mirror64[SQUARE_120_TO_64(board->getEnPassantSquare())]);
    }

    for (squareIndex = 0; squareIndex < 64; squareIndex++) {
		tempPiecesArray[squareIndex] = board->pieceAt(SQUARE_64_TO_120(mirror64[squareIndex]));
    }

    board->reset();

	for (squareIndex = 0; squareIndex < 64; squareIndex++) {
        tp = swapPiece[tempPiecesArray[squareIndex]];
        board->pieceAt(SQUARE_64_TO_120(squareIndex)) = tp;
    }

	board->setSide(tempSide);
	board->setCastlePermission(tempCastlePerm);
	board->setEnPassantSquare(tempEnPas);

	board->setPositionKey(board->generatePositionKey());

	board->updateListsMaterial();

    ASSERT(board->check());
}
