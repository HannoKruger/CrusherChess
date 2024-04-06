#pragma once
#include <cstring>
#include "moves.h"

enum { ALL_MOVES, ONLY_CAPTURES };

//version 1


#define copy_board()                                                 \
U64 bitboards_copy[12], occupancies_copy[3];                         \
int enpassant_copy, castle_copy;                                     \
memcpy(bitboards_copy, bitboards, 96);                               \
memcpy(occupancies_copy, occupancies, 24);						     \
enpassant_copy = enpassant, castle_copy = castle;                    \
U64 hash_key_copy = hash_key;

#define take_back()                                                  \
memcpy(bitboards, bitboards_copy, 96);                               \
memcpy(occupancies, occupancies_copy, 24);                           \
side ^= 1, enpassant = enpassant_copy, castle = castle_copy;		 \
hash_key = hash_key_copy;



 void generate_moves(Moves *move_list);
 int make_move(int move);
 bool is_square_attacked(int square, int side);
 void add_move(Moves* move_list, int move);