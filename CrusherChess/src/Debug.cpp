#include <cstdio>
#include <iostream>
#include <format>
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