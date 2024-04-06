#include "random.h"
#include "zobristKeys.h"
/****************************************\
 ========================================
			 Zobrist keys
 ========================================
\****************************************/

 U64 piece_keys[12][SQUARE_NB] = {};

//Random enpassant keys
 U64 enpassant_keys[SQUARE_NB] = {};

//Random castling keys
 U64 castle_keys[16] = {};

//random side key
 U64 side_key = 0;


void init_random_keys()
{
    //1804289383;
    PRNG rng(1070372);

	//loop over piece codes
	for (int piece = P; piece <= k; piece++)
	{
		//assign random piece key
		for (int square = 0; square < SQUARE_NB; square++)
			piece_keys[piece][square] = rng.rand<U64>();
	}
	//loop over board
	for (int square = 0; square < SQUARE_NB; square++)
		enpassant_keys[square] = rng.rand<U64>();

	//init castle keys
	for (int i = 0; i < 16; i++)
		castle_keys[i] = rng.rand<U64>();

	//init random side key
	side_key = rng.rand<U64>();
}

