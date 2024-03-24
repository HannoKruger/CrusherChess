#pragma once
#include "typedefs.h"

extern bool quit; // exit engine flag
extern bool time_set; // time control availability
extern bool stopped; // when time is up
extern U64 start_time; // uci starttime command
extern U64 stop_time; // uci stoptime command


int parse_move(const char* move_string);
void parse_position(const char* command);
int get_piece_count();
void parse_go(const char* command);
void uci_loop();

int input_waiting();
void read_input();
void communicate();

//                              pieces left   0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
constexpr const int moves_by_pieces[] = { 0, 1, 3, 4, 5, 6, 8, 9, 10, 11, 13, 14, 15, 16, 18, 19, 20, 21, 23, 24, 25, 26, 28, 29, 30, 31, 33, 34, 35, 36, 38, 39, 40, 41, 43, 44, 45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 61, 63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 80 };
