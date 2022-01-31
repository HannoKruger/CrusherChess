#pragma once
#define NOMINMAX
#include <iostream>
#include <cassert>
#include <string>
#include <fcntl.h>
#include <io.h>
#include <map>
#include <chrono>
#include <wtypes.h>
#include <vector>


//#include <windows.h>


typedef unsigned long long U64;
//move list
typedef struct Moves
{
    int moves[255];
    //could possibily use this?
    //std::size(data);

   // int legal[255];

    //move count
    int count;

} Moves;

// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
constexpr auto killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "


#pragma warning(disable:6031)

/****************************************\
 ========================================
                Methods
 ========================================
\****************************************/
#pragma region
void print_bitboard(U64 bitboard);
void print_magic_numbers();
void calc_mask_files();
U64 mask_pawn_attacks(bool side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 bishop_blocker_attacks(int square, U64 block);
U64 rook_blocker_attacks(int square, U64 block);
void init_leapers_attacks();
void init_all();

static inline unsigned int randu32();
static inline U64 randu64();

U64 generate_magic_number();

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
U64 find_magic_number(int square, int relevant_bits, int bishop);
void init_magic_numbers();
void init_sliders_attacks(int bishop);

static inline U64 get_bishop_attacks(int square, U64 occupancy);
static inline U64 get_rook_attacks(int square, U64 occupancy);
static inline bool is_square_attacked(int square, int side);


//void search_position(int depth);
#pragma endregion

/****************************************\
 ========================================
                  Enums
 ========================================
\****************************************/
#pragma region
//piece cords
enum Square : int
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,

    SQUARE_NB, NO_SQ
};

//enum Square : int
//{
//    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
//    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
//    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
//    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
//    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
//    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
//    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
//    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
//    SQ_NONE,
//
//    SQUARE_ZERO = 0,
//    SQUARE_NB = 64
//};

//clors
enum Colors { WHITE, BLACK, BOTH };

enum { ROOK, BISHOP };

//castling rights represented in 4 bits
enum Castling { WK = 1, WQ = 2, BK = 4, BQ = 8 };

// encode pieces
enum Pieces { P, N, B, R, Q, K, p, n, b, r, q, k };
//globals
enum { COLOR_NB = 2 };


#pragma endregion
