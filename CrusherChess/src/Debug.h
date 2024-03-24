#pragma once
#include "moves.h"

void print_bitboard(unsigned long long int bitboard);
void print_magic_numbers();
void print_move_list(Moves* move_list);
void test_move_encoding();
void print_moves_scores(Moves* moves);
void print_attacked_squares(int side);
std::string get_move_string(int move);