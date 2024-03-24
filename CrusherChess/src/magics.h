#pragma once

 #include "typedefs.h"

U64 get_bishop_attacks(int square, U64 occupancy);
 U64 get_rook_attacks(int square, U64 occupancy);
 U64 get_queen_attacks(int square, U64 occupancy);

void init_sliders_attacks(int piece);
U64 find_magic_number(int square, int relevant_bits, int bishop);
void init_magic_numbers();