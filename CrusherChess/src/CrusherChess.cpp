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
#include <vector>
#include <ostream>


#include "types.h"
#include "movegen.h"
#include "magics.h"
#include "transposition.h"
#include "zobristkeys.h"
#include "uci.h"



#pragma warning(disable:6031)


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

   
	parse_fen(start_position);
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