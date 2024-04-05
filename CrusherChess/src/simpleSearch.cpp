#include <cstdio>
#include <iostream>
#include <vector>
#include "Debug.h"
#include "movegen.h"
#include "types.h"
#include "simpleSearch.h"

int Mposition_evaluate(int cnt, int depth)
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


int MMiniMax(int depth, bool Maximizing)
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

