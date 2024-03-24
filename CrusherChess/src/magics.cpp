#include <string>
#include <cstring>
#include "magics.h"
#include "random.h"
#include "bitoperations.h"
#include "types.h"

/****************************************\
 ========================================
				Magics
 ========================================
\****************************************/
#pragma region Magics
// set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
	// occupancy map
	U64 occupancy = 0ULL;

	// loop over the range of bits within attack mask
	for (int count = 0; count < bits_in_mask; count++)
	{
		// get 1st bit index of attacks mask
		int square = get_lsb_index(attack_mask);

		// pop 1st bit in attack map
		pop_bit(attack_mask, square);

		// make sure occupancy is on board
		if (index & (1 << count))
			// populate occupancy map
			occupancy |= (1ULL << square);
	}
	// return occupancy map
	return occupancy;
}

//find magic number
U64 find_magic_number(int square, int relevant_bits, int bishop)
{
	const int size = 4096;
    PRNG rng(1070372);

	U64 occupancies[size], attacks[size], used_attacks[size];
	U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

	//init occupancy indicies
	int occupancy_indices = 1 << relevant_bits;

	for (int i = 0; i < occupancy_indices; i++)
	{
		occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);

		attacks[i] = bishop ? bishop_blocker_attacks(square, occupancies[i]) :
			rook_blocker_attacks(square, occupancies[i]);
	}

	for (int random_cnt = 0; random_cnt < 100000000; random_cnt++)
	{
		//generate magic number candidate
		U64 magic_number = rng.sparse_rand<U64>();

		//skip bad numbers
		if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

		//init used attacks
		memset(used_attacks, 0ULL, sizeof(used_attacks));

		int i, fail;
		for (i = 0, fail = 0; !fail && i < occupancy_indices; i++)
		{
			//init magic index
			int magic_index = (int)((occupancies[i] * magic_number) >> (64 - relevant_bits));

			//if magic index works
			if (used_attacks[magic_index] == 0ULL)
			{
				//init used attacks
				used_attacks[magic_index] = attacks[i];
			}
			else if (used_attacks[magic_index] != attacks[i])
			{
				//magic index doesnt work
				fail = 1;
			}
		}
		if (!fail)
			return magic_number;
	}
	//doesnt work
	printf(" Magic number fails!\n");
	return 0ULL;
}


void init_magic_numbers()
{
	throw std::string("magic numbers not defined");

	/* for (int square = 0; square < SQUARE_NB; square++)
		 rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
	 for (int square = 0; square < SQUARE_NB; square++)
		 bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);*/
}

void init_sliders_attacks(int piece)
{
	for (int square = 0; square < 64; square++)
	{
		//init bishop & rook masks
		bishop_masks[square] = mask_bishop_attacks(square);
		rook_masks[square] = mask_rook_attacks(square);

		//init current mask
		U64 attack_mask = (piece == BISHOP) ? bishop_masks[square] : rook_masks[square];

		//init occupancy bit count
		int relevant_bits_count = count_bits(attack_mask);

		//init occupancy indicies
		int occupancy_indicies = (1 << relevant_bits_count);

		//loop over occupancy
		for (int i = 0; i < occupancy_indicies; i++)
		{
			if (piece == BISHOP)
			{
				//init current occupoancy
				U64 occupancy = set_occupancy(i, relevant_bits_count, attack_mask);
				//init magic index
				int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
				//init bishop attacks
				bishop_attacks[square][magic_index] = bishop_blocker_attacks(square, occupancy);
			}
			else//rook
			{
				//init current occupoancy
				U64 occupancy = set_occupancy(i, relevant_bits_count, attack_mask);
				//init magic index
				int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
				//init rook attacks
				rook_attacks[square][magic_index] = rook_blocker_attacks(square, occupancy);
			}
		}
	}
}

//on the fly
 U64 get_bishop_attacks(int square, U64 occupancy)
{
	//get bishop attacks assuming current board occupancy
	occupancy &= bishop_masks[square];
	occupancy *= bishop_magic_numbers[square];
	occupancy >>= 64 - bishop_relevant_bits[square];

	return bishop_attacks[square][occupancy];
}
//on the fly
 U64 get_rook_attacks(int square, U64 occupancy)
{
	//get rook attacks assuming current board occupancy
	occupancy &= rook_masks[square];
	occupancy *= rook_magic_numbers[square];
	occupancy >>= 64 - rook_relevant_bits[square];

	//print_bitboard(occupancy);

	return rook_attacks[square][occupancy];
}
//on the fly
 U64 get_queen_attacks(int square, U64 occupancy)
{
	return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}


#pragma endregion