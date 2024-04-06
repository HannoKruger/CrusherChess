#include <cassert>
#include <iostream>
#include "transposition.h"
#include "types.h"

U64 hash_entries = 0ULL;
TT* hash_table = nullptr;

// dynamically allocate memory for hash table
void init_hash_table(int mb)
{
	const U64 ONE_MB = 0x100000ULL;

	// init hash size
	U64 hash_size = ONE_MB * mb;

	// init number of hash entries
	hash_entries = hash_size / sizeof(TT);

	// free hash table if not empty
	if (hash_table != nullptr)
    {
        std::cout << "Clearing hash memory..." << custom_endl;

		free(hash_table);
	}

	// allocate memory
	hash_table = (TT*)malloc(hash_entries * sizeof(TT));

	// if allocation has failed
	if (hash_table == nullptr)
	{
        std::cout << "Couldn't allocate memory for hash table, trying " << mb / 2 << "MB... " << custom_endl;

		// try to allocate with half size
		init_hash_table(mb / 2);
	}
	// if allocation succeeded
	else
	{
		// clear hash table
		clear_hash_table();
        std::cout << "Hash table is initialized with " << hash_entries << " entries" << custom_endl;
	}
}

// clear TT (hash table)
void clear_hash_table()
{
	// init hash table entry pointer
	TT* hash_entry;

	// loop over TT elements
	for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry++)
	{
		// reset TT inner fields
		hash_entry->key = 0;
		hash_entry->depth = 0;
		hash_entry->flag = 0;
		hash_entry->score = 0;
	}

	//this propably wont work for dynamic array
	//memset(hash_table, 0, sizeof(hash_table));

	assert(hash_table[0].depth == 0);
	assert(hash_table[0].flag == 0);
	assert(hash_table[0].key == 0);
	assert(hash_table[0].score == 0);
}

 int read_hash_entry(int alpha, int beta, int depth)
{
	//bring hash key down to table index storing the score data for current board position if available
	TT* hash_entry = &hash_table[hash_key % hash_entries];

	//make sure we're dealing with correct position
	if (hash_entry->key == hash_key)
	{
		//make sure to match depth our search is at
		if (hash_entry->depth >= depth)
		{
			int score = hash_entry->score;

			//retrieve score independant from actual path
			//from root position to current position
			if (score <= -mate_score) score += ply;
			if (score >= mate_score) score -= ply;

			//match the exact score (PV node)
			if (hash_entry->flag == hash_flag_exact)
				return score;
			//match alpha score (fail low node)
			if (hash_entry->flag == hash_flag_alpha && score <= alpha)
				return alpha;
			//match beta score (fail high node)
			if (hash_entry->flag == hash_flag_beta && score >= beta)
				return beta;

		}
	}
	//entry doesnt exist
	return no_hash_entry;
}

 void write_hash_entry(int score, int depth, int hash_flag)
{
	//bring hash key down to table index storing the score data for current board position if available
	TT* hash_entry = &hash_table[hash_key % hash_entries];

	//adjust score to find mate in shortest path
	//store score independant from actual path
	//from root position to current position
	if (score <= -mate_score) score -= ply;
	if (score >= mate_score) score += ply;

	//write hash entry data
	hash_entry->key = hash_key;
	hash_entry->score = score;
	hash_entry->flag = hash_flag;
	hash_entry->depth = depth;
}