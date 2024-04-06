#pragma once

#include "typedefs.h"

#define get_bit(bitboard,square) (((bitboard) >> (square)) & 1ULL)
#define set_bit(bitboard,square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard,square) ((bitboard) &= ~(1ULL << (square)))

/*
		  binary move bits              name        BitCount   hex constants

	0000 0000 0000 0000 0011 1111   source square       6      0x3f
	0000 0000 0000 1111 1100 0000   target square       6      0xfc0
	0000 0000 1111 0000 0000 0000   piece               4      0xf000
	0000 1111 0000 0000 0000 0000   promoted piece      4      0xf0000
	0001 0000 0000 0000 0000 0000   capture flag        1      0x100000
	0010 0000 0000 0000 0000 0000   double push flag    1      0x200000
	0100 0000 0000 0000 0000 0000   enpassant flag      1      0x400000
	1000 0000 0000 0000 0000 0000   castling flag       1      0x800000
*/

#define encode_move(SOURCE, TARGET, PIECE, PROMOTED_PIECE, CAPTURE, DOUBLE_PAWN, ENPASSANT, CASTLING)\
(( (SOURCE)             ) |  \
 ( (TARGET)         << 6  ) |  \
 ( (PIECE)          << 12 ) |  \
 ( (PROMOTED_PIECE) << 16 ) |  \
 ( (CAPTURE)        << 20 ) |  \
 ( (DOUBLE_PAWN)    << 21 ) |  \
 ( (ENPASSANT)      << 22 ) |  \
 ( (CASTLING)       << 23 ))


#define get_move_source(move)    (((move) & 0x3f)           )
#define get_move_target(move)    (((move) & 0xfc0)     >>  6)
#define get_move_piece(move)     (((move) & 0xf000)    >> 12)
#define get_move_promoted(move)  (((move) & 0xf0000)   >> 16)
//flags
#define get_move_capture(move)    ((move) & 0x100000)
#define get_move_double(move)     ((move) & 0x200000)
#define get_move_enpassant(move)  ((move) & 0x400000)
#define get_move_castling(move)   ((move) & 0x800000)


 int count_bits(U64 num);
 int get_lsb_index(U64 num);
 int count_bitsv1(U64 num);