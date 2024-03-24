//#define DEBUG

//includes
#pragma region

#define NOMINMAX

#include <stack>
#include <iostream>
#include <cassert>
#include <string>
#include <io.h>
#include <chrono>
#include <wtypes.h>
#include <vector>
#include <ostream>


#include "types.h"
#include "movegen.h"
#include "magics.h"
#include "transposition.h"
#include "zobristkeys.h"
#include "uci.h"


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


#pragma warning(disable:6031)



/****************************************\
 ========================================
		   Debug + Input & output
 ========================================
\****************************************/
#pragma region

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

void print_board(bool ascii = false)
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

/****************************************\
 ========================================
			 Initialization
 ========================================
\****************************************/
#pragma region
void init_leapers_attacks()
{
	for (int square = 0; square < SQUARE_NB; ++square)
	{
		//Pawn attacks
		pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
		pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);

		//Knight attacks
		knight_attacks[square] = mask_knight_attacks(square);

		//King attacks
		king_attacks[square] = mask_king_attacks(square);
	}
}

//set file or rank
U64 set_file_rank_mask(int file_no, int rank_no)
{
	U64 mask = 0ULL;

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++, square++)
		{

			if (file == file_no || rank == rank_no)
			{
				//printf("file: %d   rank:%d\n", file, rank);
				mask |= set_bit(mask, square);
			}
		}
	}

	return mask;
}

//init evaluation masks
void init_evaluation_masks()
{
	//use -1 to not set a file
	U64 mask = set_file_rank_mask(-1, -1);

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++, square++)
		{
			file_masks[square] |= set_file_rank_mask(file, -1);
			rank_masks[square] |= set_file_rank_mask(-1, rank);

			isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
			isolated_masks[square] |= set_file_rank_mask(file + 1, -1);



			passed_masks[WHITE][square] |= set_file_rank_mask(file - 1, -1);
			passed_masks[WHITE][square] |= set_file_rank_mask(file, -1);
			passed_masks[WHITE][square] |= set_file_rank_mask(file + 1, -1);

			passed_masks[BLACK][square] |= passed_masks[WHITE][square];
			passed_masks[BLACK][square] |= passed_masks[WHITE][square];
			passed_masks[BLACK][square] |= passed_masks[WHITE][square];

			//loop over redundant ranks and reset redudant bits 
			for (int i = 0; i < (8 - rank); i++)
			{
				passed_masks[WHITE][square] &= ~rank_masks[(7 - i) * 8 + file];
				passed_masks[BLACK][square] &= ~rank_masks[i * 8 + file];
			}

			//print_bitboard(passed_masks[WHITE][square]);
		}
	}
}


void init_all()
{
	init_leapers_attacks();
	init_sliders_attacks(BISHOP);
	init_sliders_attacks(ROOK);
	init_random_keys();
	init_evaluation_masks();
	//init hash table with default 64mb
	init_hash_table(64);

    throw new std::runtime_error("Not implemented");
	//parse_fen(start_position);
}
#pragma endregion

/****************************************\
 ========================================
				 Perft
 ========================================
\****************************************/
#pragma region

//std::set<U64> keys;
//std::vector<U64> collisions;


//leaf nodes (number of positions reached at given depth
U64 nodes = 0ULL;

static inline void perft_driver(int depth)
{
	//escape
	if (depth <= 0)
	{
		nodes++;
		return;
	}

	Moves move_list[1];
	generate_moves(move_list);

	for (int move_count = 0; move_count < move_list->count; move_count++)
	{
		// preserve board state
		copy_board();


		// make move
		if (!make_move(move_list->moves[move_count]))
		{
			take_back();
			continue;
		}

		//auto t = keys.insert(hash_key);

		//if (t.second)
		//{
			//does not exist in set
		//}
		//else
		//{
			//collisions.push_back(hash_key);
			//std::cout << "Collision! " << "length:" << keys.size() << custom_endl;

			//print_board();
		//}

		// call perft driver recursively
		perft_driver(depth - 1);

		// take back
		take_back();
	}
}

void perft_test(int depth)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	Moves move_list[1];
	generate_moves(move_list);

	if (depth > 0)
	{
		for (int move_count = 0; move_count < move_list->count; move_count++)
		{
			// preserve board state
			copy_board();

			auto move = move_list->moves[move_count];

			// make move
			if (!make_move(move))
			{
				take_back();
				continue;
			}

			//auto t = keys.insert(hash_key);

			//if (t.second)
			//{
				//does not exist in set
			//}
			//else
			//{
				//std::cout << "Collision! " << "length:" << keys.size() << custom_endl;
			//	collisions.push_back(hash_key);

				//print_board();
			//}

			U64 cumulative_nodes = nodes;

			// call perft driver recursively
			perft_driver(depth - 1);

			U64 current_nodes = nodes - cumulative_nodes;

			// take back
			take_back();

            const int promoted = get_move_promoted(move);
            std::cout <<
            square_to_coordinates[get_move_source(move)] <<
            square_to_coordinates[get_move_target(move)] <<
            (promoted ? promoted_pieces.at(promoted) : ' ') <<
            " nodes: " << current_nodes << custom_endl;
		}
	}
	else nodes++;

	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	auto elapsedMS = ((long double)elapsedMicro) / 1000;
	auto elapsedS = elapsedMS / 1000;

	printf("\nDepth:%d\n", depth);
	printf("Nodes:%llu\n", nodes);
	printf("Total Time:%.2LF (MS)\n", elapsedMS);
	//printf("Seconds:%.2LF (S)\n", elapsedS);
	printf("Nodes per second:%.0LF\n\n", (nodes / elapsedS));
}

#pragma endregion
/****************************************\
 ========================================
			   Simple Search
 ========================================
\****************************************/
#pragma region

const int piece_scores[12] = { 10, 30, 31, 50, 90, 2000, -10, -30, -31, -50, -90, -2000 };


static inline int Mposition_evaluate(int cnt, int depth)
{

	/*if (bitboards[K] == 0 || bitboards[k] == 0)
	{
		print_board();
		printf("Errr king missing");
		std::cin.get();
		assert(0 && "King is missing");
	}*/

	/*if (bitboards[K] == 0)
		return 30000;
	if (bitboards[k] == 0)
		return -30000;*/





	if (cnt == 0)
	{
		//printf("count was zero!!!!\n");


		if (is_square_attacked(get_lsb_index(bitboards[K]), BLACK))//white king attacked by black
		{
			//printf("white is mate\n");

			if (depth <= 0)
				return -2000;

			return -2000 * depth;
		}

		if (is_square_attacked(get_lsb_index(bitboards[k]), WHITE))//black king attacked by white
		{
			//printf("black is mate\n");

			if (depth <= 0)
				return 2000;

			return 2000 * depth;
		}

		//printf("stalemate\n");
		return 0;

	}


	int score = 0;


	for (int i = P; i <= k; i++)
		score += ((count_bits(bitboards[i]) + depth) * piece_scores[i]);
	//printf("score:%d\n",score);
	return score;
}


static inline int MMiniMax(int depth, bool Maximizing)
{
	Moves moves[1];
	generate_moves(moves);

	int cnt = 0;
	for (int i = 0; i < moves->count; i++)
	{
		copy_board();
		if (!make_move(moves->moves[i]))
		{
			take_back();
			continue;
		}
		take_back();

		//moves->legal[i] = moves->moves[i];

		//legal[cnt++] = moves->moves[i];	
		cnt++;
	}

	if (depth == 0 || cnt == 0)
		return Mposition_evaluate(cnt, depth);

	if (Maximizing)//white
	{
		int maxeval = INT_MIN;

		for (int i = 0; i < moves->count; i++)
		{
			copy_board();

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}


			int eval = MMiniMax(depth - 1, false);

			take_back();


			maxeval = std::max(maxeval, eval);
		}
		return maxeval;
	}
	else//black
	{
		int mineval = INT_MAX;

		for (int i = 0; i < moves->count; i++)
		{
			copy_board();

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, true);

			take_back();


			mineval = std::min(mineval, eval);
		}
		return mineval;
	}
}


////not working
//int legal[255];
//
//static inline int MiniMaxV2(int depth, bool Maximizing)
//{
//	Moves moves[1];
//	generate_moves(moves);
//
//	int cnt = 0;
//	for (int i = 0; i < moves->count; i++)
//	{
//		copy_board();
//		if (!make_move(moves->moves[i], ALL_MOVES))
//		{
//			take_back();
//			continue;
//		}
//		take_back();
//
//		legal[cnt] = moves->moves[i];
//		cnt++;
//	}
//
//	if (depth == 0 || cnt == 0)
//		return position_evaluate(cnt,depth);
//
//	if (Maximizing)//white
//	{
//		int maxeval = INT_MIN;
//
//		for (int i = 0; i < cnt; i++)
//		{
//			copy_board();
//			
//			make_move(legal[i], ALL_MOVES);
//
//			int eval = MiniMaxV2(depth - 1, false);
//
//			take_back();
//
//
//			maxeval = std::max(maxeval, eval);
//		}
//		return maxeval;
//	}
//	else//black
//	{
//		int mineval = INT_MAX;
//
//		for (int i = 0; i < cnt; i++)
//		{
//			copy_board();
//		
//			make_move(legal[i], ALL_MOVES);
//
//			int eval = MiniMaxV2(depth - 1, true);
//
//			take_back();
//
//
//			mineval = std::min(mineval, eval);
//		}
//		return mineval;
//	}
//}


int Mget_best_move(int depth)
{
	Moves moves[1];
	generate_moves(moves);

	std::vector<int> bestmoves;

	printf("\nCount: %d\n", (moves->count));

	if (side == WHITE)
	{
		int bestScore = INT_MIN;

		for (int i = 0; i < moves->count; i++)//paralize this
		{
			copy_board();

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, false);

            std::cout << "no" << (i + 1) << "  score: " << eval << "  move:";
			std::cout << get_move_string(moves->moves[i]) << custom_endl;

			take_back();

			if (eval >= bestScore)
			{
				if (eval == bestScore)
					bestmoves.push_back(moves->moves[i]);
				else
				{
					printf("White new eval:%d\n", eval);
					bestmoves.clear();
					bestmoves.push_back(moves->moves[i]);
					bestScore = eval;
				}
			}
		}
	}
	else
	{
		int bestScore = INT_MAX;

		for (int i = 0; i < moves->count; i++)//paralize this
		{
			copy_board();

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, true);
			printf("no%d  score: %d  move:", (i + 1), eval);
			std::cout << get_move_string(moves->moves[i]) << custom_endl;

			take_back();

			if (eval <= bestScore)
			{
				if (eval == bestScore)
					bestmoves.push_back(moves->moves[i]);
				else
				{
                    std::cout << "black new eval:" << eval << custom_endl;

					bestmoves.clear();
					bestmoves.push_back(moves->moves[i]);
					bestScore = eval;
				}
			}
		}
	}
	return bestmoves[rand() % bestmoves.size()];
}

void Msearch_position(int depth)
{
	printf("depth:%d\n", depth);

	int best = Mget_best_move(depth);

    std::cout << "bestmove " << get_move_string(best);
}

#pragma endregion
/****************************************\
 ========================================
				Evaluation
 ========================================
\****************************************/
#pragma region
// material scrore

/*
	♙ =   100   = ♙
	♘ =   300   = ♙ * 3
	♗ =   350   = ♙ * 3 + ♙ * 0.5
	♖ =   500   = ♙ * 5
	♕ =   1000  = ♙ * 10
	♔ =   10000 = ♙ * 100

*/



// material score [game phase][piece]
const int material_score[2][12] =
{
	// opening material score
	82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000,

	// endgame material score
	94, 281, 297, 512,  936, 12000, -94, -281, -297, -512,  -936, -12000
};

// game phase scores
const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

// game phases
enum { OPENING, ENDGAME, MIDDELGAME };



// positional piece scores [game phase][piece][square]
const int positional_score[2][6][SQUARE_NB] =

// opening positional piece scores //
{
	//pawn
	0,   0,   0,   0,   0,   0,  0,   0,
	98, 134,  61,  95,  68, 126, 34, -11,
	-6,   7,  26,  31,  65,  56, 25, -20,
	-14,  13,   6,  21,  23,  12, 17, -23,
	-27,  -2,  -5,  12,  17,   6, 10, -25,
	-26,  -4,  -4, -10,   3,   3, 33, -12,
	-35,  -1, -20, -23, -15,  24, 38, -22,
	0,   0,   0,   0,   0,   0,  0,   0,

	// knight
	-167, -89, -34, -49,  61, -97, -15, -107,
	-73, -41,  72,  36,  23,  62,   7,  -17,
	-47,  60,  37,  65,  84, 129,  73,   44,
	-9,  17,  19,  53,  37,  69,  18,   22,
	-13,   4,  16,  13,  28,  19,  21,   -8,
	-23,  -9,  12,  10,  19,  17,  25,  -16,
	-29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,

	// bishop
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	-4,   5,  19,  50,  37,  37,   7,  -2,
	-6,  13,  13,  26,  34,  12,  10,   4,
	0,  15,  15,  15,  14,  27,  18,  10,
	4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,

	// rook
	32,  42,  32,  51, 63,  9,  31,  43,
	27,  32,  58,  62, 80, 67,  26,  44,
	-5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,

	// queen
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	-9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	-1, -18,  -9,  10, -15, -25, -31, -50,

	// king
	-65,  23,  16, -15, -56, -34,   2,  13,
	29,  -1, -20,  -7,  -8,  -4, -38, -29,
	-9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,


	// Endgame positional piece scores //

	//pawn
	0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	94, 100,  85,  67,  56,  53,  82,  84,
	32,  24,  13,   5,  -2,   4,  17,  17,
	13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	4,   7,  -6,   1,   0,  -5,  -1,  -8,
	13,   8,   8,  10,  13,   0,   2,  -7,
	0,   0,   0,   0,   0,   0,   0,   0,

	// knight
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,

	// bishop
	-14, -21, -11,  -8, -7,  -9, -17, -24,
	-8,  -4,   7, -12, -3, -13,  -4, -14,
	2,  -8,   0,  -1, -2,   6,   0,   4,
	-3,   9,  12,   9, 14,  10,   3,   2,
	-6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,

	// rook
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	7,  7,  7,  5,  4,  -3,  -5,  -3,
	4,  3, 13,  1,  2,   1,  -1,   2,
	3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,

	// queen
	-9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,

	// king
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	10,  17,  23,  15,  20,  45,  44,  13,
	-8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};

// mirror positional score tables for opposite side only flip vertical
const int mirror_square[SQUARE_NB] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

//// mirror positional score tables for opposite side (flips vertical and horizontal)
//const int mirror_square[SQUARE_NB] =
//{
//	h1, g1, f1, e1, d1, c1, b1, a1, 
//	h2, g2, f2, e2, d2, c2, b2, a2, 
//	h3, g3, f3, e3, d3, c3, b3, a3, 
//	h4, g4, f4, e4, d4, c4, b4, a4, 
//	h5, g5, f5, e5, d5, c5, b5, a5, 
//	h6, g6, f6, e6, d6, c6, b6, a6, 
//	h7, g7, f7, e7, d7, c7, b7, a7, 
//	h8, g8, f8, e8, d8, c8, b8, a8
//};


/*
		  Rank mask            File mask           Isolated mask        Passed pawn mask
		for square a6        for square f2         for square g2          for square c4
	8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 1 0 0    8  0 0 0 0 0 1 0 1     8  0 1 1 1 0 0 0 0
	7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 1 0 0    7  0 0 0 0 0 1 0 1     7  0 1 1 1 0 0 0 0
	6  1 1 1 1 1 1 1 1    6  0 0 0 0 0 1 0 0    6  0 0 0 0 0 1 0 1     6  0 1 1 1 0 0 0 0
	5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 1 0 0    5  0 0 0 0 0 1 0 1     5  0 1 1 1 0 0 0 0
	4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 1 0 0    4  0 0 0 0 0 1 0 1     4  0 0 0 0 0 0 0 0
	3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 1 0 0    3  0 0 0 0 0 1 0 1     3  0 0 0 0 0 0 0 0
	2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 1 0 0    2  0 0 0 0 0 1 0 1     2  0 0 0 0 0 0 0 0
	1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 1 0 0    1  0 0 0 0 0 1 0 1     1  0 0 0 0 0 0 0 0
	   a b c d e f g h       a b c d e f g h       a b c d e f g h        a b c d e f g h
*/



//// file masks [square]
//U64 file_masks[SQUARE_NB];
//
//// rank masks [square]
//U64 rank_masks[SQUARE_NB];
//
//// isolated pawn masks [square]
//U64 isolated_masks[SQUARE_NB];
//
//// white passed pawn masks [square]
//U64 white_passed_masks[SQUARE_NB];
//
//// black passed pawn masks [square]
//U64 black_passed_masks[SQUARE_NB];




// extract rank from a square [square]
const int get_rank[SQUARE_NB] =
{
	7, 7, 7, 7, 7, 7, 7, 7,
	6, 6, 6, 6, 6, 6, 6, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

// double pawns penalty
const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;

// isolated pawn penalty
const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

// bonusses
const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 };

// semi open file score
const int semi_open_file_score = 10;

// open file score
const int open_file_score = 15;

// mobility units (values from engine Fruit reloaded)
static const int bishop_unit = 4;
static const int queen_unit = 9;

// mobility bonuses (values from engine Fruit reloaded)
static const int bishop_mobility_opening = 5;
static const int bishop_mobility_endgame = 5;
static const int queen_mobility_opening = 1;
static const int queen_mobility_endgame = 2;

// king's shield bonus
const int king_shield_bonus = 5;


static inline int get_game_phase_score()
{
	/*
		The game phase score of the game is derived from the pieces
		(not counting pawns and kings) that are still on the board.
		The full material starting position game phase score is:

		4 * knight material score in the opening +
		4 * bishop material score in the opening +
		4 * rook material score in the opening +
		2 * queen material score in the opening
	*/

	// white & black game phase scores
	int white_piece_scores = 0, black_piece_scores = 0;

	// loop over white pieces
	for (int piece = N; piece <= Q; piece++)
		white_piece_scores += count_bits(bitboards[piece]) * material_score[OPENING][piece];

	// loop over black pieces
	for (int piece = n; piece <= q; piece++)
		black_piece_scores += count_bits(bitboards[piece]) * -material_score[OPENING][piece];

	// return game phase score
	return white_piece_scores + black_piece_scores;
}

static inline int lerp_positional_score(int piece_type,int square,int game_phase_score)
{
	return
		(
		positional_score[OPENING][piece_type][square] * game_phase_score +
		positional_score[ENDGAME][piece_type][square] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}
static inline int lerp_material_score(int piece, int game_phase_score)
{
	return
		(material_score[OPENING][piece] * game_phase_score
		+ material_score[ENDGAME][piece] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}


static inline int evaluate()
{
	int game_phase_score = get_game_phase_score();
	//printf("game phase score: %d\n", game_phase_score);
	 

	//static evaluation
	int score = 0,opening_score = 0,endgame_score = 0;

	//current pieces bitboard copy
	U64 bitboard;

	//init piece and square
	int piece, square;

	//penalties
	int double_pawns = 0;


	for (int bb_piece = P; bb_piece <= k; bb_piece++)
	{
		bitboard = bitboards[bb_piece];

		while (bitboard)
		{
			piece = bb_piece;
			square = get_lsb_index(bitboard);

			// get opening/endgame material score
			opening_score += material_score[OPENING][piece];
			endgame_score += material_score[ENDGAME][piece];

			if (piece < 6)//white
			{
				opening_score += positional_score[OPENING][piece][square];
				endgame_score += positional_score[ENDGAME][piece][square];
			}
			else		 //black    //using -6 might be faster here
			{
				opening_score -= positional_score[OPENING][piece_to_type[piece]][mirror_square[square]];
				endgame_score -= positional_score[ENDGAME][piece_to_type[piece]][mirror_square[square]];
			}

			
		    /*old method:
			//calculate the interpolated material score between opening and endgame
			if (game_phase == MIDDELGAME)		
				score += lerp_material_score(piece, game_phase_score);		
			//material score
			else
				score += material_score[game_phase][piece];	


			//calculate the interpolated positional score between opening and endgame
			if (game_phase == MIDDELGAME)
			{
				if (piece < 6)//white
					score += lerp_positional_score(piece, square, game_phase_score);
				else		  //black	 //-6 might be faster here
					score -= lerp_positional_score(piece_to_type[piece], mirror_square[square], game_phase_score);
			}
			// score material weights with pure scores in OPENING or ENDGAME
			else
			{
				if (piece < 6)//white
					score += positional_score[game_phase][piece][square];
				else		  //black    //-6 might be faster here
					score -= positional_score[game_phase][piece_to_type[piece]][mirror_square[square]];
			}
			*/


			
				
			switch (piece)
			{
				//evaluate white pawns
				case P:
				
					// double pawn penalty
					double_pawns = count_bits(bitboards[P] & file_masks[square]);

					// on double pawns (tripple, etc)
					if (double_pawns > 1)
					{
						opening_score += (double_pawns - 1) * double_pawn_penalty_opening;
						endgame_score += (double_pawns - 1) * double_pawn_penalty_endgame;
					}

					// on isolated pawn
					if ((bitboards[P] & isolated_masks[square]) == 0)
					{
						// give an isolated pawn penalty
						opening_score += isolated_pawn_penalty_opening;
						endgame_score += isolated_pawn_penalty_endgame;
					}
					// on passed pawn
					if ((passed_masks[WHITE][square] & bitboards[p]) == 0)
					{
						// give passed pawn bonus
						opening_score += passed_pawn_bonus[get_rank[square]];
						endgame_score += passed_pawn_bonus[get_rank[square]];
					}
					break;
				//evaluate white knights
				case N:
				
					break;
				//evaluate white bishops
				case B:
					// mobility
					opening_score += (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_opening;
					endgame_score += (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_endgame;
					break;
				//evaluate white rooks
				case R:
					// semi open file
					if ((bitboards[P] & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score += semi_open_file_score;
						endgame_score += semi_open_file_score;
					}

					// semi open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score += open_file_score;
						endgame_score += open_file_score;
					}
					break;
				//evaluate white queens
				case Q:
					// mobility
					opening_score += (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_opening;
					endgame_score += (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_endgame;
					break;
				//evaluate white king
				case K:
					
					// open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add open file penalty
						opening_score -= open_file_score;
						endgame_score -= open_file_score;
					}
					// semi open file
					else if ((bitboards[P] & file_masks[square]) == 0)
					{
						// add semi open file penalty
						opening_score -= semi_open_file_score;
						endgame_score -= semi_open_file_score;
					}

					// king safety bonus
					opening_score += count_bits(king_attacks[square] & occupancies[WHITE]) * king_shield_bonus;
					endgame_score += count_bits(king_attacks[square] & occupancies[WHITE]) * king_shield_bonus;

					break;
				//evaluate black pawns
				case p:
					// double pawn penalty
					double_pawns = count_bits(bitboards[p] & file_masks[square]);

					// on double pawns (tripple, etc)
					if (double_pawns > 1)
					{
						opening_score -= (double_pawns - 1) * double_pawn_penalty_opening;
						endgame_score -= (double_pawns - 1) * double_pawn_penalty_endgame;
					}

					// on isolated pawn
					if ((bitboards[p] & isolated_masks[square]) == 0)
					{
						// give an isolated pawn penalty
						opening_score -= isolated_pawn_penalty_opening;
						endgame_score -= isolated_pawn_penalty_endgame;
					}
					// on passed pawn
					if ((passed_masks[BLACK][square] & bitboards[P]) == 0)
					{
						// give passed pawn bonus
						opening_score -= passed_pawn_bonus[get_rank[square]];
						endgame_score -= passed_pawn_bonus[get_rank[square]];
					}
					break;
				//evaluate black knights
				case n:
				
					break;
					//evaluate black bishops
				case b:
					// mobility
					opening_score -= (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_opening;
					endgame_score -= (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_endgame;
					break;
				//evaluate black rooks
				case r:
					// semi open file
					if ((bitboards[p] & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score -= semi_open_file_score;
						endgame_score -= semi_open_file_score;
					}

					// semi open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score -= open_file_score;
						endgame_score -= open_file_score;
					}
					break;
				//evaluate black queens
				case q:
					// mobility
					opening_score -= (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_opening;
					endgame_score -= (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_endgame;
					break;
				//evaluate black king
				case k:
					//open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						//open file penalty
						opening_score += open_file_score;
						endgame_score += open_file_score;
					}
					// semi open file
					else if ((bitboards[p] & file_masks[square]) == 0)
					{
						//add semi open file penalty
						opening_score += semi_open_file_score;
						endgame_score += semi_open_file_score;
					}		

					//king safety bonus
					opening_score -= count_bits(king_attacks[square] & occupancies[BLACK]) * king_shield_bonus;
					endgame_score -= count_bits(king_attacks[square] & occupancies[BLACK]) * king_shield_bonus;
					break;
			}
			

			//pop lsb bit
			pop_bit(bitboard, square);
		}
	}


	/*
				Now in order to calculate interpolated score
				for a given game phase we use this formula
				(same for material and positional scores):

				(
				  score_opening * game_phase_score +
				  score_endgame * (opening_phase_score - game_phase_score)
				) / opening_phase_score

				E.g. the score for pawn on d4 at phase say 5000 would be
				interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
			*/



			//int game_phase;

			////game > opening
			//if (game_phase_score > opening_phase_score) game_phase = OPENING;
			////game < endgame
			//else if (game_phase_score < endgame_phase_score) game_phase = ENDGAME;
			////game between opening and endgame 
			//else game_phase = MIDDELGAME;


	// return pure opening score in opening
	if (game_phase_score > opening_phase_score) score = opening_score;
	// return pure endgame score in engame
	else if (game_phase_score < endgame_phase_score) score = endgame_score;
	// interpolate score in the middlegame
	else
	 score = (
		 opening_score * game_phase_score +
		 endgame_score * (opening_phase_score - game_phase_score)
		 ) / opening_phase_score;


	return (side == WHITE) ? score : -score;
}

#pragma endregion

/****************************************\
 ========================================
				Search
 ========================================
\****************************************/
#pragma region

/*
   most valuable victim & less valuable attacker

	(Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
		Pawn   105    205    305    405    505    605
	  Knight   104    204    304    404    504    604
	  Bishop   103    203    303    403    503    603
		Rook   102    202    302    402    502    602
	   Queen   101    201    301    401    501    601
		King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};



//interval for when to communicate with gui
#define LISTEN 16383 //32767 //2047


//killer moves [id][ply]
int killer_moves[2][MAX_PLY] = {};

//history moves [piece][square]
int history_moves[12][SQUARE_NB] = {};


/*
	  ================================
			Triangular PV table
	  --------------------------------
		PV line: e2e4 e7e5 g1f3 b8c6
	  ================================
		   0    1    2    3    4    5

	  0    m1   m2   m3   m4   m5   m6

	  1    0    m2   m3   m4   m5   m6

	  2    0    0    m3   m4   m5   m6

	  3    0    0    0    m4   m5   m6

	  4    0    0    0    0    m5   m6

	  5    0    0    0    0    0    m6
*/

// PV length [ply[
int pv_length[MAX_PLY] = {};

// PV table [ply][ply]
int pv_table[MAX_PLY][MAX_PLY] = {};

int follow_pv = 0, score_pv = 0;

int move_scores[255] = {};


static inline void enable_pv_scoring(Moves* move_list)
{
	//disable
	follow_pv = FALSE;

	//loop over moves
	for (int i = 0; i < move_list->count; i++)
	{
		//make sure we hit pv
		if (pv_table[0][ply] == move_list->moves[i])
		{
			//enable move scoring
			score_pv = TRUE;
			//enable pv
			follow_pv = TRUE;
		}
	}
}

/*
* ===================
*    Move ordering
* ===================
*
* 1 PV move
* 2 Captures and MVA/LVA
* 3 1st killer move
* 4 2nd killer move
* 5 History moves
* 6 Unsorted moves
*/

static inline int score_move(int move)
{
	//if PV scoring allowed
	if (score_pv)
	{
		if (pv_table[0][ply] == move)
		{
			//disable flag
			score_pv = FALSE;

			//printf("PV move: ");
            //std::cout << get_move_string(move);
			//printf(" ply: %d\n", ply);

			//score PV move to search first
			return 20000;
		}
	}


	if (get_move_capture(move))
	{
		int target_piece = P;

		int start_piece, end_piece;

		int target_square = get_move_target(move);

		//white to move
		if (side == WHITE) { start_piece = p; end_piece = k; }
		//black to move
		else { start_piece = P;	end_piece = K; }

		//loop only over oppsite side pieces
		for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
		{
			//if piece on target square remove it
			if (get_bit(bitboards[bb_piece], target_square))
			{
				target_piece = bb_piece;
				break;
			}
		}
		//score capture
		return mvv_lva[get_move_piece(move)][target_piece] + 10000;
	}
	else//score quiet move
	{
		//score ist killer move
		if (killer_moves[0][ply] == move)
			return 9000;
		//score 2nd killer  move
		else if (killer_moves[1][ply] == move)
			return 8000;
		//score history move
		else
			return history_moves[get_move_piece(move)][get_move_target(move)];
	}

	return 0;
}

static inline void sort_moves(Moves* move_list)
{
	for (int i = 0; i < move_list->count; i++)
		move_scores[i] = score_move(move_list->moves[i]);

	//bubble sort
	for (int i = 0; i < move_list->count; i++)
	{
		for (int j = i + 1; j < move_list->count; j++)
		{
			if (move_scores[i] < move_scores[j])
			{
				//swap scores
				std::swap(move_scores[i], move_scores[j]);
				//swap moves
				std::swap(move_list->moves[i], move_list->moves[j]);
			}
		}
	}
}

//repetition detection
static inline bool is_repetition()
{
	//loop over repetition indices
	for (int i = 0; i < repetition_index; i++)
	{
		//if same hash key 
		if (repetition_table[i] == hash_key)
			return true;
	}

	//if no repetition found
	return false;
}


//alpha = maximizing
//beta = minimizing
static inline int quiescence(int alpha, int beta)
{
	//every time to listen to gui / user
	if ((nodes & LISTEN) == 0)
		communicate();

	nodes++;


	//make sure no overflow on arrays
	if (ply > MAX_PLY - 1)
	{
		assert(!"ply greater than max ply");
		return evaluate();
	}

	int eval = evaluate();


	//fails high
	if (eval >= beta)
		return beta;
	//better move
	if (eval > alpha)
	{
		//PV node (Prinsipal variation)
		alpha = eval;
	}


	Moves moves[1];
	generate_moves(moves);

	sort_moves(moves);//sort!

	for (int i = 0; i < moves->count; i++)
	{
		if (!get_move_capture(moves->moves[i]))
			continue;


		copy_board();

		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		if (!make_move(moves->moves[i]))
		{
			take_back();

			ply--;
			repetition_index--;
			continue;
		}


		int score = -quiescence(-beta, -alpha);

		ply--;
		repetition_index--;
		take_back();
		//return if time is up
		if (stopped) return 0;

		//better move
		if (score > alpha)
		{
			//PV node (Prinsipal variation)
			alpha = score;

			//fails high
			if (score >= beta)
				return beta;
		}
	}
	//fails low
	return alpha;
}


const int full_depth_moves = 4;
const int reduction_limit = 3;

//main search
static inline int negamax(int alpha, int beta, int depth)
{
	//init pv length
	pv_length[ply] = ply;

	//current move score
	int score;
	int hash_flag = hash_flag_alpha;


	//return draw on repetition
	if (ply && is_repetition())
		return 0;


	//a hack to check if current node is pv or not 
	bool pv_node = beta - alpha > 1;

	//if move already searched return move score without further search
	//also make sure we are noot in the root ply and current node is not a PV node
	if (ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && !pv_node)
		return score;


	//every time to listen to gui / user
	if ((nodes & LISTEN) == 0)
		communicate();


	if (depth == 0)
		//run quisuince
		return quiescence(alpha, beta);

	//make sure no overflow on arrays
	if (ply > MAX_PLY - 1)
	{
		assert(!"ply greater than max ply");
		return evaluate();
	}

	//number of nodes
	nodes++;

	int in_check = is_square_attacked(
		(side == WHITE ? get_lsb_index(bitboards[K]) :
			get_lsb_index(bitboards[k])), side ^ 1);


	if (in_check) depth++;//search further when in check

	//legal moves counter
	int legal_count = 0;


	//Null move pruning
	if (depth >= 3 && !in_check && ply)
	{
		copy_board();

		//increment ply
		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		//hash enpassant if available
		if (enpassant != NO_SQ) hash_key ^= enpassant_keys[enpassant];

		//reset enpassant
		enpassant = NO_SQ;

		//switch side: giving opponent extra move
		side ^= 1;

		//hash side
		hash_key ^= side_key;

		//search with reduced depth (depth - 1 - r  where r = reduction_limit)
		score = -negamax(-beta, -beta + 1, depth - 1 - 2);

		//revert ply
		ply--;
		repetition_index--;

		take_back();

		//return if time is up
		if (stopped) return 0;

		//fail hard cutoff
		if (score >= beta)
			return beta;//fail high

	}

	Moves moves[1];
	generate_moves(moves);

	//we are now following pv line
	if (follow_pv)
		enable_pv_scoring(moves);

	sort_moves(moves);//sort!

	int moves_searched = 0;

	for (int i = 0; i < moves->count; i++)
	{
		copy_board();

		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		if (!make_move(moves->moves[i]))
		{
			take_back();
			ply--;
			repetition_index--;

			continue;
		}
		legal_count++;


		//LMR

		//full depth search
		if (moves_searched == 0)
			score = -negamax(-beta, -alpha, depth - 1);
		else//late move reduction LMR
		{
			//condition for LMR
			if (
				moves_searched >= full_depth_moves &&
				depth >= reduction_limit &&
				!in_check &&
				!get_move_capture(moves->moves[i]) &&
				!get_move_promoted(moves->moves[i])
				)
			{
				//search current move with reduced depth
				score = -negamax(-alpha - 1, -alpha, depth - 2);
			}
			else
			{
				//hack to ensure full depth search
				score = alpha + 1;
			}
			//principle variation search PVS
			if (score > alpha)
			{
				score = -negamax(-alpha - 1, -alpha, depth - 1);

				//if LMR fails re-search at full depth
				if (score > alpha && score < beta)
					score = -negamax(-beta, -alpha, depth - 1);
			}

		}

		ply--;
		repetition_index--;
		take_back();
		//return if time is up
		if (stopped) return 0;

		moves_searched++;

		//better move
		if (score > alpha)
		{
			//switch hash flag from fail low to one for PV node
			hash_flag = hash_flag_exact;


			//only quite moves
			if (!get_move_capture(moves->moves[i]))
			{
				//store history moves
				history_moves[get_move_piece(moves->moves[i])][get_move_target(moves->moves[i])] += depth;
			}

			//PV node (Prinsipal variation)
			alpha = score;

			//write PV move
			pv_table[ply][ply] = moves->moves[i];


			//loop over next ply
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
			{
				//copy move from deeper ply into curent
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			//adjust pv length
			pv_length[ply] = pv_length[ply + 1];


			//fails high (hard)
			if (score >= beta)
			{
				//store hash entry with score equal to beta
				write_hash_entry(beta, depth, hash_flag_beta);

				//only quite moves
				if (!get_move_capture(moves->moves[i]))
				{
					//store killer
					killer_moves[1][ply] = killer_moves[0][ply];
					killer_moves[0][ply] = moves->moves[i];
				}

				return beta;
			}
		}
	}
	//no legal moves
	if (legal_count == 0)
	{
		if (in_check)
			return -mate_value + ply;
		else//stalemate
			return 0;
	}

	//store hash entry with the score equal to alpha
	write_hash_entry(alpha, depth, hash_flag);

	//fails low
	return alpha;
}

void search_position(int depth)
{
	int previous_best = 0;
	int score = 0;

	//nodes = running total
	nodes = 0ULL;

	//reset stop flag
	stopped = false;

	//reset flags
	follow_pv = FALSE;
	score_pv = 0;

	//clear arrays
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));


	//1281490
	 //770190

	//845015
	// 
	//depth 6 tricky position
	// 
	//pre best:	1056080
	//new best:  824253
	//new best:  363479

	int alpha = -infinity;
	int beta = infinity;

	//iterative deepening
	for (int current_depth = 1; current_depth <= depth; current_depth++)
	{
		//enable pv
		follow_pv = TRUE;

		//search best
		score = negamax(alpha, beta, current_depth);

		//we fell outside the window, try again with full window (and same depth)
		if ((score <= alpha) || (score >= beta))
		{
			alpha = -infinity;
			beta = infinity;
            --current_depth;
			continue;
		}

		//set up the window for the next itteration
		alpha = score - 50;
		beta = score + 50;

		//get best move so far if time is up
		if (stopped) break;
		previous_best = pv_table[0][0];

		//send info to gui
		std::cout << "info ";

		if (score > -mate_value && score < -mate_score)
            std::cout << "score mate " << -(score + mate_value) / 2;
		else if (score > mate_score && score < mate_value)
            std::cout << "score mate " << (mate_value - score) / 2;
		else
            std::cout << "score cp " << score;

        std::cout <<
        " depth " <<current_depth <<
        " nodes " << nodes <<
        " time " << (GetTickCount64() - start_time) <<
        " pv ";

		//loop over pv line
		for (int i = 0; i < pv_length[0]; i++)
            std::cout << get_move_string(pv_table[0][i]) << " ";
		std::cout << custom_endl;

		if (time_set && GetTickCount64() + 4 > stop_time)
		{
			//stopped = true; 	

			if (ply != 0)
				std::cout << "Ply err! ply->" << ply << custom_endl;

			assert(ply == 0);

			break;
		}
	}
	
	std::string m;
	m += "bestmove ";
	//if the search was stopped, and we have a previous best move
	if (stopped && previous_best)
		m += get_move_string(previous_best);
	//else if the search was not stopped, or we don't have a previous best move return current
	else
		m += get_move_string(pv_table[0][0]);

	std::cout << m << custom_endl;
}

#pragma endregion
/****************************************\
 ========================================
				 Main
 ========================================
\****************************************/


//#define DEBUG

int main()
{ 
	//setvbuf(stdout, NULL, _IONBF, 0);
	//std::cout.setf(std::ios_base::unitbuf);

	//use %ls for wchar

	//need to fix score imbalance with mirror table eroor in this position "1k6/8/8/8/8/8/8/6K1 w - - 0 1"

	//bugs:
	//lose on time
	//null move from pv, is this fixed now?

	init_all();

#ifdef DEBUG
		//tricky
		//parse_fen("r3k2r/p1ppqpb1/1n2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R w KQkq - 0 1 ");
        parse_fen("6Br/PP2k2P/6K1/8/nn6/b1n3n1/pppppp1p/4b3 w - - 0 1");
		//parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");
		//parse_fen(start_position);

		print_board();

		printf("score:%d\n", evaluate());

        std::cout << "\nRunning perft" << custom_endl;
		perft_test(2);
		
		//auto startTime = std::chrono::high_resolution_clock::now();

		//search_position(10);
		//make_move(pv_table[0][0]);

		//search_position(10);
		//make_move(pv_table[0][0]);
	
		//search_position(10);
		//make_move(pv_table[0][0]);

		//search_position(10);
		//make_move(pv_table[0][0]);

		//print_board();

		//auto endTime = std::chrono::high_resolution_clock::now();

		//auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//auto elapsedMS = ((long double)elapsedMicro) / 1000;
		//auto elapsedS = elapsedMS / 1000;
	
		//printf("Total Time:%.2LF (MS)\n", elapsedMS);
		//printf("Seconds:%.2LF (S)\n\n", elapsedS);





	 //   startTime = std::chrono::high_resolution_clock::now();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();
		//
		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e6d5");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e6d5 c3d5");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		//print_board();


	 //   endTime = std::chrono::high_resolution_clock::now();

		//elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//elapsedMS = ((long double)elapsedMicro) / 1000;
		//elapsedS = elapsedMS / 1000;

		//printf("Total Time:%.2LF (MS)\n", elapsedMS);
		//printf("Seconds:%.2LF (S)\n\n", elapsedS);

		
		//getchar();

		std::wcin.get();
#else
		uci_loop();
#endif

	free(hash_table);
	return 0;
}