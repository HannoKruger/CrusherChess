#include "types.h"

// Define global variables
U64 nodes = 0ULL;

U64 bitboards[12] = {};
U64 occupancies[3] = {};
int side = WHITE;
int enpassant = NO_SQ;
int castle = 0;
U64 hash_key = 0ULL;
U64 repetition_table[1000] = {};
int repetition_index = 0;
int ply = 0;

U64 pawn_attacks[COLOR_NB][SQUARE_NB] = {};
U64 knight_attacks[SQUARE_NB] = {};
U64 king_attacks[SQUARE_NB] = {};

//attacks masks
U64 bishop_masks[SQUARE_NB] = {};
U64 rook_masks[SQUARE_NB] = {};

//attacks table [square][occupancies]
U64 bishop_attacks[SQUARE_NB][512] = {};
U64 rook_attacks[SQUARE_NB][4096] = {};

//masks for pawns [square]
 U64 file_masks[SQUARE_NB] = {};
 U64 rank_masks[SQUARE_NB] = {};

 U64 isolated_masks[SQUARE_NB] = {};
 U64 passed_masks[COLOR_NB][SQUARE_NB] = {};