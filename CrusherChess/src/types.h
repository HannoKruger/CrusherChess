#pragma once
#include <map>
#include <cstdio>
#include <ostream>
#include <iostream>
#include "typedefs.h"
#include "Debug.h"
#include "bitoperations.h"
#include "platform_defs.h"

using namespace CrusherChess;

#define MAX_HASH 32768
//max reachable ply
#define MAX_PLY 64

inline constexpr const char* VERSION = "1.3.1";

//leaf nodes (number of positions reached at given depth
extern U64 nodes;


// FEN dedug positions
#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
constexpr auto killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "


template <typename CharT, typename Traits>
 std::basic_ostream<CharT, Traits>& custom_endl(std::basic_ostream<CharT, Traits>& os) {
    return std::cout << std::endl;
}


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

// piece types
enum PieceTypes { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

//castling rights represented in 4 bits
enum Castling { WK = 1, WQ = 2, BK = 4, BQ = 8 };

// encode pieces
enum Pieces { P, N, B, R, Q, K, p, n, b, r, q, k };
//globals
enum { COLOR_NB = 2 };


#pragma endregion





/****************************************\
 ========================================
				 Globals
 ========================================
\****************************************/
#pragma region
//piece bitboards
extern U64 bitboards[12];

//occupancy bitboards
extern U64 occupancies[3];

//side to move, init to white
extern int side;

//en passant square
extern int enpassant;

//castling rights
extern int castle;

//"almost" unique position identifier aka hash key
extern U64 hash_key;

extern U64 repetition_table[1000];//number of plies in whole game
extern int repetition_index;
//half move counter
extern int ply;


//leaper attacks
extern U64 pawn_attacks[COLOR_NB][SQUARE_NB];
extern U64 knight_attacks[SQUARE_NB];
extern U64 king_attacks[SQUARE_NB];

//attacks masks
extern U64 bishop_masks[SQUARE_NB];
extern U64 rook_masks[SQUARE_NB];

//attacks table [square][occupancies]
extern U64 bishop_attacks[SQUARE_NB][512];
extern U64 rook_attacks[SQUARE_NB][4096];

//masks for pawns [square]
extern U64 file_masks[SQUARE_NB];
extern U64 rank_masks[SQUARE_NB];

extern U64 isolated_masks[SQUARE_NB];
extern U64 passed_masks[COLOR_NB][SQUARE_NB];

#pragma endregion





/****************************************\
 ========================================
				Constants
 ========================================
\****************************************/
#pragma region
// ASCII pieces
constexpr const char* ascii_pieces = "PNBRQKpnbrqk";

// unicode pieces
// wchar_t* unicode_pieces[12] = { L"♙", L"♘", L"♗", L"♖", L"♕", L"♔", L"♟︎", L"♞", L"♝", L"♜", L"♛", L"♚" };
//unicode piece with code point
// unicode pieces
//\u265A

//FE0E is a variation selector-15, it forces the black and white pieces to be displayed as text, not as emoji
#ifdef WINDOWS
constexpr const wchar_t* unicode_pieces[12] = { L"\u265F\uFE0E", L"\u265E\uFE0E", L"\u265D\uFE0E", L"\u265C\uFE0E", L"\u265B\uFE0E", L"\u265A\uFE0E",  L"\u2659\uFE0E", L"\u2658\uFE0E", L"\u2657\uFE0E", L"\u2656\uFE0E",L"\u2655\uFE0E", L"\u2654\uFE0E" };
#else
//constexpr const char* unicode_pieces[12] = { "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚" };
constexpr const char* unicode_pieces[12] = { "\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A", "\u2659", "\u2658", "\u2657", "\u2656","\u2655", "\u2654" };
#endif


//convert ascii character pieces to constants
 const std::map <char, int> char_pieces =
{
	//white pieces
	{'P',P},
	{'N',N},
	{'B',B},
	{'R',R},
	{'Q',Q},
	{'K',K},
	//black pieces
	{'p',p},
	{'n',n},
	{'b',b},
	{'r',r},
	{'q',q},
	{'k',k}
};

//promoted pieces for uci
 const std::map <int,const char> promoted_pieces =
{
	//white pieces
	{Q,'q'},
	{R,'r'},
	{B,'b'},
	{N,'n'},

	//black pieces
	{q,'q'},
	{r,'r'},
	{b,'b'},
	{n,'n'},
};

constexpr const int piece_to_type[12] = { PAWN,KNIGHT,BISHOP,ROOK,QUEEN,KING,PAWN,KNIGHT,BISHOP,ROOK,QUEEN,KING };

constexpr const U64 not_a_file = 18374403900871474942ULL;
constexpr const U64 not_h_file = 9187201950435737471ULL;
constexpr const U64 not_hg_file = 4557430888798830399ULL;
constexpr const U64 not_ab_file = 18229723555195321596ULL;

//Occupancy bit count per square (total attacks) counted with masked_attacks

//bishop bit count
constexpr const int bishop_relevant_bits[64] =
{
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};
// rook bit count
constexpr const int rook_relevant_bits[64] =
{
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

// convert squares to coordinates
constexpr const char* square_to_coordinates[] =
{
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

//magic numbers
constexpr const U64 rook_magic_numbers[64] =
{
 0x8a80104000800020ULL,
 0x140002000100040ULL,
 0x2801880a0017001ULL,
 0x100081001000420ULL,
 0x200020010080420ULL,
 0x3001c0002010008ULL,
 0x8480008002000100ULL,
 0x2080088004402900ULL,
 0x800098204000ULL,
 0x2024401000200040ULL,
 0x100802000801000ULL,
 0x120800800801000ULL,
 0x208808088000400ULL,
 0x2802200800400ULL,
 0x2200800100020080ULL,
 0x801000060821100ULL,
 0x80044006422000ULL,
 0x100808020004000ULL,
 0x12108a0010204200ULL,
 0x140848010000802ULL,
 0x481828014002800ULL,
 0x8094004002004100ULL,
 0x4010040010010802ULL,
 0x20008806104ULL,
 0x100400080208000ULL,
 0x2040002120081000ULL,
 0x21200680100081ULL,
 0x20100080080080ULL,
 0x2000a00200410ULL,
 0x20080800400ULL,
 0x80088400100102ULL,
 0x80004600042881ULL,
 0x4040008040800020ULL,
 0x440003000200801ULL,
 0x4200011004500ULL,
 0x188020010100100ULL,
 0x14800401802800ULL,
 0x2080040080800200ULL,
 0x124080204001001ULL,
 0x200046502000484ULL,
 0x480400080088020ULL,
 0x1000422010034000ULL,
 0x30200100110040ULL,
 0x100021010009ULL,
 0x2002080100110004ULL,
 0x202008004008002ULL,
 0x20020004010100ULL,
 0x2048440040820001ULL,
 0x101002200408200ULL,
 0x40802000401080ULL,
 0x4008142004410100ULL,
 0x2060820c0120200ULL,
 0x1001004080100ULL,
 0x20c020080040080ULL,
 0x2935610830022400ULL,
 0x44440041009200ULL,
 0x280001040802101ULL,
 0x2100190040002085ULL,
 0x80c0084100102001ULL,
 0x4024081001000421ULL,
 0x20030a0244872ULL,
 0x12001008414402ULL,
 0x2006104900a0804ULL,
 0x1004081002402ULL
};
constexpr const U64 bishop_magic_numbers[64] =
{
 0x40040844404084ULL,
 0x2004208a004208ULL,
 0x10190041080202ULL,
 0x108060845042010ULL,
 0x581104180800210ULL,
 0x2112080446200010ULL,
 0x1080820820060210ULL,
 0x3c0808410220200ULL,
 0x4050404440404ULL,
 0x21001420088ULL,
 0x24d0080801082102ULL,
 0x1020a0a020400ULL,
 0x40308200402ULL,
 0x4011002100800ULL,
 0x401484104104005ULL,
 0x801010402020200ULL,
 0x400210c3880100ULL,
 0x404022024108200ULL,
 0x810018200204102ULL,
 0x4002801a02003ULL,
 0x85040820080400ULL,
 0x810102c808880400ULL,
 0xe900410884800ULL,
 0x8002020480840102ULL,
 0x220200865090201ULL,
 0x2010100a02021202ULL,
 0x152048408022401ULL,
 0x20080002081110ULL,
 0x4001001021004000ULL,
 0x800040400a011002ULL,
 0xe4004081011002ULL,
 0x1c004001012080ULL,
 0x8004200962a00220ULL,
 0x8422100208500202ULL,
 0x2000402200300c08ULL,
 0x8646020080080080ULL,
 0x80020a0200100808ULL,
 0x2010004880111000ULL,
 0x623000a080011400ULL,
 0x42008c0340209202ULL,
 0x209188240001000ULL,
 0x400408a884001800ULL,
 0x110400a6080400ULL,
 0x1840060a44020800ULL,
 0x90080104000041ULL,
 0x201011000808101ULL,
 0x1a2208080504f080ULL,
 0x8012020600211212ULL,
 0x500861011240000ULL,
 0x180806108200800ULL,
 0x4000020e01040044ULL,
 0x300000261044000aULL,
 0x802241102020002ULL,
 0x20906061210001ULL,
 0x5a84841004010310ULL,
 0x4010801011c04ULL,
 0xa010109502200ULL,
 0x4a02012000ULL,
 0x500201010098b028ULL,
 0x8040002811040900ULL,
 0x28000010020204ULL,
 0x6000020202d0240ULL,
 0x8918844842082200ULL,
 0x4010011029020020ULL
};

/*
						   castling   move     in      in
							  right update     binary  decimal
									  BBWW
 king & rooks didn't move:     1111 & 1111  =  1111    15
		white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13

		 black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7
*/

// castling rights update constants
constexpr const int castling_rights[64] =
{
	 7, 15, 15, 15,  3, 15, 15, 11,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	13, 15, 15, 15, 12, 15, 15, 14
};


#pragma endregion


/****************************************\
 ========================================
			  Leaper attacks
 ========================================
\****************************************/
#pragma region
//generate not a file, not h file and so on
inline void calc_mask_files()
{
	U64 not_a_file = 0ULL;
	U64 not_h_file = 0ULL;


	for (size_t y = 0, cnt = 0; y < 8; ++y)
	{
		for (size_t x = 0; x < 8; x++, ++cnt)
		{
			if (x != 0)
				set_bit(not_a_file, cnt);
			if (x != 7)
				set_bit(not_h_file, cnt);
		}
	}
	printf("not a\n");
	print_bitboard(not_a_file);
	printf("not h\n");
	print_bitboard(not_h_file);
}
//generate pawn attacks
inline U64 mask_pawn_attacks(bool side, int square)
{
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;

	set_bit(bitboard, square);

	//white
	if (side == WHITE)
	{
		auto shift7 = bitboard >> 7;
		auto shift9 = bitboard >> 9;


		if (shift7 & not_a_file) attacks |= shift7;
		if (shift9 & not_h_file) attacks |= shift9;
	}
	//black
	else
	{
		auto shift7 = bitboard << 7;
		auto shift9 = bitboard << 9;


		if (shift7 & not_h_file) attacks |= shift7;
		if (shift9 & not_a_file) attacks |= shift9;
	}

	return attacks;
}
//generate knight attacks
inline U64 mask_knight_attacks(int square)
{
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;

	set_bit(bitboard, square);

	auto TR = bitboard >> 15;//top right
	auto TL = bitboard >> 17;//top left
	auto RT = bitboard >> 6;//right top
	auto LT = bitboard >> 10;//left top

	auto BL = bitboard << 15;//bottom left
	auto BR = bitboard << 17;//bottom right
	auto LB = bitboard << 6;//left bottom
	auto RB = bitboard << 10;//right bottom


	if (TR & not_a_file)  attacks |= TR;
	if (TL & not_h_file)  attacks |= TL;
	if (RT & not_ab_file) attacks |= RT;
	if (LT & not_hg_file) attacks |= LT;

	if (BL & not_h_file)  attacks |= BL;
	if (BR & not_a_file)  attacks |= BR;
	if (LB & not_hg_file) attacks |= LB;
	if (RB & not_ab_file) attacks |= RB;

	return attacks;
}
//generate king attacks
inline U64 mask_king_attacks(int square)
{
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;

	set_bit(bitboard, square);

	auto TL = bitboard >> 9;//top left
	auto T = bitboard >> 8;//top
	auto TR = bitboard >> 7;//top right
	auto L = bitboard >> 1;//left

	auto BR = bitboard << 9;//botom right
	auto B = bitboard << 8;//botom
	auto BL = bitboard << 7;//botom left
	auto R = bitboard << 1;//right

	if (TL & not_h_file)  attacks |= TL;
	if (TR & not_a_file) attacks |= TR;
	if (L & not_h_file) attacks |= L;

	if (BR & not_a_file) attacks |= BR;
	if (BL & not_h_file)  attacks |= BL;
	if (R & not_a_file) attacks |= R;

	attacks |= T;
	attacks |= B;

	return attacks;
}
#pragma endregion
/****************************************\
 ========================================
   Sliding attacks for magic bitboard
 ========================================
\****************************************/
#pragma region
//doesnt go to board edge
inline U64 mask_bishop_attacks(int square)
{
	U64 attacks = 0ULL;

	//target file(x) & rank(y)
	int tx = square % 8, ty = square / 8;

	for (int x = tx + 1, y = ty + 1; x <= 6 && y <= 6; ++x, ++y) attacks |= (1ULL << (y * 8 + x));
	for (int x = tx + 1, y = ty - 1; x <= 6 && y >= 1; ++x, --y) attacks |= (1ULL << (y * 8 + x));
	for (int x = tx - 1, y = ty + 1; x >= 1 && y <= 6; --x, ++y) attacks |= (1ULL << (y * 8 + x));
	for (int x = tx - 1, y = ty - 1; x >= 1 && y >= 1; --x, --y) attacks |= (1ULL << (y * 8 + x));

	return attacks;
}
inline U64 mask_rook_attacks(int square)
{
	U64 attacks = 0ULL;

	//target file(x) & rank(y)
	int tx = square % 8, ty = square / 8;

	for (int x = tx + 1; x <= 6; ++x) attacks |= (1ULL << (ty * 8 + x));
	for (int x = tx - 1; x >= 1; --x) attacks |= (1ULL << (ty * 8 + x));
	for (int y = ty + 1; y <= 6; ++y) attacks |= (1ULL << (y * 8 + tx));
	for (int y = ty - 1; y >= 1; --y) attacks |= (1ULL << (y * 8 + tx));

	return attacks;
}

//attacks on the fly, goes to bord edge with blocker
inline U64 bishop_blocker_attacks(int square, U64 block)
{
	U64 attacks = 0ULL;
	int x, y;
	//target file(x) & rank(y)
	int tx = square % 8, ty = square / 8;

	for (x = tx + 1, y = ty + 1; x <= 7 && y <= 7; ++x, ++y)
	{
		attacks |= (1ULL << (y * 8 + x));
		if (block & (1ULL << (y * 8 + x))) break;
	}
	for (x = tx + 1, y = ty - 1; x <= 7 && y >= 0; ++x, --y)
	{
		attacks |= (1ULL << (y * 8 + x));
		if (block & (1ULL << (y * 8 + x))) break;
	}
	for (x = tx - 1, y = ty + 1; x >= 0 && y <= 7; --x, ++y)
	{
		attacks |= (1ULL << (y * 8 + x));
		if (block & (1ULL << (y * 8 + x))) break;
	}
	for (x = tx - 1, y = ty - 1; x >= 0 && y >= 0; --x, --y)
	{
		attacks |= (1ULL << (y * 8 + x));
		if (block & (1ULL << (y * 8 + x))) break;
	}
	return attacks;
}
inline U64 rook_blocker_attacks(int square, U64 block)
{
	U64 attacks = 0ULL;

	//target file(x) & rank(y)
	int tx = square % 8, ty = square / 8;

	for (int x = tx + 1; x <= 7; ++x)
	{
		attacks |= (1ULL << (ty * 8 + x));
		if (block & (1ULL << (ty * 8 + x))) break;
	}
	for (int x = tx - 1; x >= 0; --x)
	{
		attacks |= (1ULL << (ty * 8 + x));
		if (block & (1ULL << (ty * 8 + x))) break;
	}
	for (int y = ty + 1; y <= 7; ++y)
	{
		attacks |= (1ULL << (y * 8 + tx));
		if (block & (1ULL << (y * 8 + tx))) break;
	}
	for (int y = ty - 1; y >= 0; --y)
	{
		attacks |= (1ULL << (y * 8 + tx));
		if (block & (1ULL << (y * 8 + tx))) break;
	}

	return attacks;
}
#pragma endregion