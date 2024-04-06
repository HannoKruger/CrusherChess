#pragma once
#include "types.h"

//Random piece keys [piece][square]
extern U64 piece_keys[12][SQUARE_NB];

//Random enpassant keys
extern U64 enpassant_keys[SQUARE_NB];

//Random castling keys
extern U64 castle_keys[16];

//random side key
extern U64 side_key;

void init_random_keys();