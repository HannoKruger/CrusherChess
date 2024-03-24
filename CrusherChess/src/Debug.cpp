#include  <windows.h>
#include <cstdio>
#include <iostream>
#include <format>
#include <cassert>
#include <sstream>

#include "Debug.h"
#include "moves.h"
#include "bitoperations.h"
#include "movegen.h"
#include "types.h"



//for uci
std::string get_move_string(int move)
{
	if (get_move_promoted(move))
		return std::format("{}{}{}",
			square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)],
			promoted_pieces.at(get_move_promoted(move)));
	else
		return std::format("{}{}",
			square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)]);
}

void print_move_list(Moves* move_list)
{
	// do nothing on empty move list
	if (!move_list->count)
	{
		printf("No move in the moves list!\n");
		return;
	}

	printf("move   piece    capture  double  enpass  castling\n\n");

	// loop over moves within a move list
	for (int move_count = 0; move_count < move_list->count; move_count++)
	{
		// init move
		int move = move_list->moves[move_count];

		// print move
		printf("%s%s%c    %c        %d        %d       %d       %d\n", square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)],
			get_move_promoted(move) ? ascii_pieces[get_move_promoted(move)] : ' ',
			ascii_pieces[get_move_piece(move)],
			get_move_capture(move) ? 1 : 0,
			get_move_double(move) ? 1 : 0,
			get_move_enpassant(move) ? 1 : 0,
			get_move_castling(move) ? 1 : 0);
	}
	// print total number of moves
	printf("\nTotal number of moves: %d\n\n", move_list->count);
}

void test_move_encoding()
{
	int move = encode_move(e2, e4, K, Q, 1, 1, 1, 1);

	printf("src square:%s\n", square_to_coordinates[get_move_source(move)]);
	printf("target square:%s\n", square_to_coordinates[get_move_target(move)]);
	printf("Piece:%c\n", ascii_pieces[get_move_piece(move)]);
	printf("Promoted:%c\n", ascii_pieces[get_move_promoted(move)]);

	printf("Capture:%s\n", (get_move_capture(move) ? "1" : "0"));
	printf("double:%s\n", (get_move_double(move) ? "1" : "0"));
	printf("enpassant:%s\n", (get_move_enpassant(move) ? "1" : "0"));
	printf("Castle:%s\n", (get_move_castling(move) ? "1" : "0"));
}

void print_moves_scores(Moves* moves)
{
    throw std::runtime_error("Function not implemented");
	std::cout << "move scores:" << custom_endl;
	for (int i = 0; i < moves->count; i++)
	{
        //std::cout << get_move_string(moves->moves[i]) << " score: " << score_move(moves->moves[i]) << custom_endl;
	}
}

void print_magic_numbers()
{
	printf("Rook:\n");
	for (int square = 0; square < SQUARE_NB; square++)
		printf(" 0x%llxULL,\n", rook_magic_numbers[square]);

	printf("\n\nBishop:\n");
	for (int square = 0; square < SQUARE_NB; square++)
		printf(" 0x%llxULL,\n", bishop_magic_numbers[square]);
}

void print_attacked_squares(int side)
{
	for (int rank = 0, square = 0; rank < 8; ++rank)
	{
		printf("%d ", 8 - rank);
		for (int file = 0; file < 8; ++file, ++square)
		{
			printf("%d ", is_square_attacked(square, side) ? 1 : 0);
		}
		printf("\n");
	}
	printf("  a b c d e f g h");
}

std::string get_fen_Stocfish_Version_not_working()
{
    throw std::runtime_error("Function not implemented");

//	int emptyCnt;
//	std::ostringstream ss;
//
//	const std::string PieceToChar("pnbrqkPNBRQK");
//
//	for (int r = 7; r >= 0; --r)
//	{
//		for (int f = 0; f <= 7; ++f)
//		{
//			for (emptyCnt = 0; f <= 7 && (get_piece(f, r) == -1); ++f)
//				++emptyCnt;
//
//			if (emptyCnt)
//				ss << emptyCnt;
//
//			if (f <= 7)
//			{
//				int piece = get_piece(f, r);
//
//				if (piece != -1)
//					ss << PieceToChar[piece];
//				else
//					ss << ' ';
//			}
//		}
//
//		if (r > 0)
//			ss << '/';
//	}
//
//	ss << (side == WHITE ? " w " : " b ");
//
//	if (castle & 1)		ss << 'K';
//	if (castle & 2)		ss << 'Q';
//	if (castle & 4)		ss << 'k';
//	if (castle & 8)		ss << 'q';
//	if (!(castle & 15))	ss << '-';
//
//	ss << (enpassant == NO_SQ ? " - " : " " + std::string(square_to_coordinates[enpassant]) + " ");
//
//	//need to add half and full move counters
//
//	return ss.str();
}


/****************************************\
 ========================================
		   Debug + Input & output
 ========================================
\****************************************/
#pragma region


class TextAttr
{
	friend std::ostream& operator<<(std::ostream& out, TextAttr attr)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr.value);
		return out;
	}
public:
	explicit TextAttr(WORD attributes) : value(attributes) {}
private:
	WORD value;
};

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)


int get_piece(int file, int rank)
{
	int piece = -1;

	for (int i = P; i <= k; i++)
	{
		if (get_bit(bitboards[i], (rank * 8 + file)))
		{
			piece = i;
			break;
		}
	}
	return piece;
}

std::string get_fen()
{
	std::ostringstream ss;

	for (int r = 0; r < 8; r++)
	{
		for (int f = 0; f < 8; f++)
		{
			int empty = 0;

			while (get_piece(f, r) == -1 && f++ < 8)
				empty++;

			if (empty)
				ss << empty;

			if (f <= 7)
			{
				int piece = get_piece(f, r);

				if (piece == -1)
					assert(!"piece is -1");

				ss << ascii_pieces[piece];
			}
		}

		if (r < 7)
			ss << '/';
	}

	ss << (side == WHITE ? " w " : " b ");

	if (castle & 1)		ss << 'K';
	if (castle & 2)		ss << 'Q';
	if (castle & 4)		ss << 'k';
	if (castle & 8)		ss << 'q';
	if (!(castle & 15))	ss << '-';

	ss << (enpassant == NO_SQ ? " - " : " " + std::string(square_to_coordinates[enpassant]) + " ");

	//need to add half and full move counters
	ss << "0 " << "1";

	return ss.str();
}

void print_bitboard(U64 bitboard)
{
	wprintf(L"    Bitboard: %llu\n", bitboard);
	for (int rank = 0, i = 0; rank < 8; ++rank)
	{
		wprintf(L" %d  ", 8 - rank);

		for (int file = 0; file < 8; ++file, ++i)
			wprintf(L"%llu ", get_bit(bitboard, i));

		wprintf(L"\n");
	}
	//print board files
	wprintf(L"    a b c d e f g h\n\n");
	//print bitboard as unsigned number
}




void print_board(bool ascii)
{
	printf("\n");
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written = 0;

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		std::cout << TextAttr(FOREGROUND_WHITE);
		printf("%d ", 8 - rank);

		for (int file = 0; file < 8; file++, square++)
		{
			int piece = -1;

			//loop over all the pieces
			for (int bb_piece = P; bb_piece <= k; bb_piece++)
			{
				if (get_bit(bitboards[bb_piece], square))
					piece = bb_piece;
			}

			((square + rank + 1) % 2) ? std::cout << TextAttr(BACKGROUND_WHITE) : std::cout << TextAttr(FOREGROUND_WHITE);

			if (piece != -1)
			{
				if (ascii)
					printf("%c ", ascii_pieces[piece]);
				else
				{
					if ((square + rank + 1) % 2)
					{
						if (piece > 5)
							piece -= 6;
						else
							piece += 6;
					}

					//WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);

//					if (piece == P)
//						WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);
//					else
					{
						WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);
						WriteConsoleW(handle, L" ", 1, &written, NULL);
					}
				}
			}
			else
				printf("  ");
		}
		std::cout << TextAttr(FOREGROUND_WHITE);
		printf("\n");
	}

	printf("  a b c d e f g h\n\n");

	printf("Side: %s\n", side ? "Black" : "White");

	printf("En Passant: %s\n", (enpassant != NO_SQ) ? (square_to_coordinates[enpassant]) : "no");

	printf("Castling: %c%c%c%c\n",
		(castle & WK) ? 'K' : '-',
		(castle & WQ) ? 'Q' : '-',
		(castle & BK) ? 'k' : '-',
		(castle & BQ) ? 'q' : '-'
	);

	std::cout << "Fen: " << get_fen() << custom_endl;

	printf("Key: %llx\n\n", hash_key);
}



#pragma endregion
