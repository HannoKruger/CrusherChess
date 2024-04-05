#include <chrono>
#include "perft.h"
#include "types.h"
#include "movegen.h"


void perft_driver(int depth)
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