#pragma once
#include "zobristkeys.h"


/****************************************\
 ========================================
				Hasing
 ========================================
\****************************************/

//generate almost unique ID
U64 generate_hash_key()
{
	U64 final_key = 0ULL;
	U64 bitboard;

	for (int piece = P; piece <= k; piece++)
	{
		bitboard = bitboards[piece];

		//loop over pieces
		while (bitboard)
		{
			int square = get_lsb_index(bitboard);

			//hash piece
			final_key ^= piece_keys[piece][square];

			//pop lsb
			pop_bit(bitboard, square);
		}
	}
	//hash enpasant
	if (enpassant != NO_SQ)//Can remove this?
		final_key ^= enpassant_keys[enpassant];
	//hash castle
	final_key ^= castle_keys[castle];

	if (side == BLACK)
		final_key ^= side_key;

	return final_key;
}