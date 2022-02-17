//includes
#pragma region 

#pragma once

#define NOMINMAX

#include <stack>
#include <iostream>
#include <cassert>
#include <string>
#include <fcntl.h>
#include <io.h>
#include <map>
#include <chrono>
#include <wtypes.h>
#include <vector>
#include <ostream>
#include <Windows.h>
#include <consoleapi2.h>

class TextAttr
{
	friend std::ostream& operator<<(std::ostream& out, TextAttr attr)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr.value);
		return out;
	}
public:
	explicit TextAttr(WORD attributes) : value(attributes) {}
private:
	WORD value;
};

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)




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

#pragma endregion

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
/****************************************\
 ========================================
			  Bit operations
 ========================================
\****************************************/
#pragma region
//bit operations
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


//count bitboard bits
static inline int count_bitsv1(U64 num)//slow
{
	int cnt = 0;

	while (num)
	{
		num &= num - 1;
		++cnt;
	}
	return cnt;
}
static inline int count_bits(U64 num)//fast
{
	return std::popcount(num);
}


static inline int get_lsb_index(U64 num)//can be sped up
{
	//-num = (0 - num) = (~num + 1ULL) -> flip bits & add one

	if (num)
		return count_bits((num & (0 - num)) - 1);
	else
		return -1;//return illegal index
}
#pragma endregion
/****************************************\
 ========================================
				Constants
 ========================================
\****************************************/
#pragma region
// ASCII pieces
const char* ascii_pieces = "PNBRQKpnbrqk";

// unicode pieces
//const wchar_t* unicode_pieces[12] = { L"♙", L"♘", L"♗", L"♖", L"♕", L"♔", L"♟︎", L"♞", L"♝", L"♜", L"♛", L"♚" };
//unicode piece with code point
// unicode pieces
//\u265A
const wchar_t* unicode_pieces[12] = { L"\u265F", L"\u265E", L"\u265D", L"\u265C", L"\u265B", L"\u265A",  L"\u2659", L"\u2658", L"\u2657", L"\u2656",L"\u2655", L"\u2654" };


//convert ascii character pieces to constants
std::map <char, int> char_pieces =
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
std::map <int, char> promoted_pieces =
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

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_hg_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;

//Occupancy bit count per square (total attacks) counted with masked_attacks

//bishop bit count
const int bishop_relevant_bits[64] =
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
const int rook_relevant_bits[64] =
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
const char* square_to_coordinates[] =
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
const U64 rook_magic_numbers[64] =
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
const U64 bishop_magic_numbers[64] =
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
const int castling_rights[64] = 
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
				 Globals
 ========================================
\****************************************/
#pragma region
//piece bitboards
U64 bitboards[12] = {};

//occupancy bitboards
U64 occupancies[3] = {};

//side to move, init to white
int side = WHITE;

//en passant square
int enpassant = NO_SQ;

//castling rights
int castle = 0;



//leaper attacks
U64 pawn_attacks[COLOR_NB][SQUARE_NB] = {};
U64 knight_attacks[SQUARE_NB] = {};
U64 king_attacks[SQUARE_NB] = {};

//attacks masks
U64 bishop_masks[SQUARE_NB] = {};
U64 rook_masks[SQUARE_NB] = {};

//attacks table [square][occupancies]
U64 bishop_attacks[SQUARE_NB][512] = {};
U64 rook_attacks[SQUARE_NB][4096] = {};

//psuedo random number state
unsigned int random_state = 1804289383;

#pragma endregion
/****************************************\
 ========================================
		   Debug + Input & output
 ========================================
\****************************************/
#pragma region
void print_bitboard(U64 bitboard)
{
	wprintf(L"    Bitboard: %llu\n", bitboard);
	for (int rank = 0, i = 0; rank < 8; ++rank)
	{
		wprintf(L" %d  ", 8 - rank);

		for (int file = 0; file < 8; ++file, ++i)
			wprintf(L"%llu ", get_bit(bitboard, i));

		wprintf(L"\n");
	}
	//print board files
	wprintf(L"    a b c d e f g h\n\n");
	//print bitboard as unsigned number  
}

void print_board(bool ascii = false)
{
	printf("\n");
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written = 0;

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		std::cout << TextAttr(FOREGROUND_WHITE);
		printf("%d ", 8 - rank);

		for (int file = 0; file < 8; file++, square++)
		{
			int piece = -1;

			//loop over all the pieces
			for (int bb_piece = P; bb_piece <= k; bb_piece++)
			{
				if (get_bit(bitboards[bb_piece], square))
					piece = bb_piece;
			}

			((square + rank + 1) % 2) ? std::cout << TextAttr(BACKGROUND_WHITE) : std::cout << TextAttr(FOREGROUND_WHITE);

			if (piece != -1)
			{
				if (ascii)
					printf("%c ", ascii_pieces[piece]);
				else
				{
					if ((square + rank + 1) % 2)
					{
						if (piece > 5)
							piece -= 6;
						else
							piece += 6;
					}

					//WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);

					if (piece == P)
						WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);
					else
					{
						WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);
						WriteConsoleW(handle, L" ", 1, &written, NULL);
					}
				}
			}
			else
				printf("  ");
		}
		std::cout << TextAttr(FOREGROUND_WHITE);
		printf("\n");
	}
	printf("  a b c d e f g h\n\n");
	//wprintf(L"\n  \u200Aa\u2000b\u2000c\u2000d\u2000e\u2000f\u2000g\u2000h\n");
	//wprintf(L"\u2589\u2589a\u2001b\u2001c\u2001d\u2001e\u2001f\u2001g\u2001h\n\n");

	//print side
	printf("Side: %s\n", side ? "Black" : "White");
	//print en passant
	printf("En Passant: %s\n", (enpassant != NO_SQ) ? (square_to_coordinates[enpassant]) : "no");

	printf("Castling: %c%c%c%c\n\n",
		(castle & WK) ? 'K' : '-',
		(castle & WQ) ? 'Q' : '-',
		(castle & BK) ? 'k' : '-',
		(castle & BQ) ? 'q' : '-'
	);
}

void print_attacked_squares(int side)
{
	for (int rank = 0, square = 0; rank < 8; ++rank)
	{
		printf("%d ", 8 - rank);
		for (int file = 0; file < 8; ++file, ++square)
		{
			printf("%d ", is_square_attacked(square, side) ? 1 : 0);
		}
		printf("\n");
	}
	printf("  a b c d e f g h");
}

void parse_fen(const char* fen)
{
	// reset board pos and state
	memset(bitboards, 0ULL, sizeof(bitboards));
	memset(occupancies, 0ULL, sizeof(occupancies));

	castle = 0;

	for (int rank = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			int square = rank * 8 + file, piece = -1;

			//parse chars
			if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
			{
				piece = char_pieces[*fen];
				//set piece
				set_bit(bitboards[piece], square);

				//increment pointer
				fen++;
			}
			//parse numbers
			if (*fen >= '0' && *fen <= '9')
			{
				//convert char to int
				int offset = *fen - '0';

				//if no piece on current square
				if (piece == -1)
					file--;

				file += offset;

				//increment pointer
				fen++;
			}
			//parse rank sep
			if (*fen == '/')
				fen++;
		}
	}
	if (*fen == ' ')	 fen++;//remove space
	//init side
	(*fen == 'w' || *fen == 'W') ? side = WHITE : side = BLACK;
	if (*++fen == ' ') fen++;//remove space

	//parse castling rights
	if (*fen != '-')
	{
		while (*fen != ' ')
		{
			switch (*fen++)
			{
			case  'K': castle |= WK; break;
			case  'Q': castle |= WQ; break;
			case  'k': castle |= BK; break;
			case  'q': castle |= BQ; break;
			case  '-': break;
			default: printf("fen string in inccorect format!"); break;
			}
		}
	}
	else fen++;

	if (*fen == ' ')  fen++;//remove space

	//parse en passant
	if (*fen != '-')
	{
		int file = fen[0] - 'a';
		int rank = 8 - (fen[1] - '0');

		//init enpassant square
		enpassant = rank * 8 + file;

		fen += 2;
	}
	else
	{
		enpassant = NO_SQ;
		fen++;
	}
	if (*fen == ' ')  fen++;//remove space

	//need to implement 50 move rule counters
	//printf("fen:%s\n", fen);

	//init occupancies

	for (int piece = P; piece <= K; piece++)	//white
		occupancies[WHITE] |= bitboards[piece];
	for (int piece = p; piece <= k; piece++)//black
		occupancies[BLACK] |= bitboards[piece];
	//both
	occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];
}

void print_magic_numbers()
{
	printf("Rook:\n");
	for (int square = 0; square < SQUARE_NB; square++)
		printf(" 0x%llxULL,\n", rook_magic_numbers[square]);

	printf("\n\nBishop:\n");
	for (int square = 0; square < SQUARE_NB; square++)
		printf(" 0x%llxULL,\n", bishop_magic_numbers[square]);
}

//for uci
void print_move(int move)
{
	if (get_move_promoted(move))	
		printf("%s%s%c",
		square_to_coordinates[get_move_source(move)],
		square_to_coordinates[get_move_target(move)],
		promoted_pieces[get_move_promoted(move)]
		);
	else
		printf("%s%s",
			square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)]			
		);
}

void print_move_list(Moves* move_list)
{
	// do nothing on empty move list
	if (!move_list->count)
	{
		printf("No move in the moves list!\n");
		return;
	}

	printf("move   piece    capture  double  enpass  castling\n\n");

	// loop over moves within a move list
	for (int move_count = 0; move_count < move_list->count; move_count++)
	{
		// init move
		int move = move_list->moves[move_count];

		// print move
		printf("%s%s%c    %c        %d        %d       %d       %d\n", square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)],
			get_move_promoted(move) ? ascii_pieces[get_move_promoted(move)] : ' ',
			ascii_pieces[get_move_piece(move)],
			get_move_capture(move) ? 1 : 0,
			get_move_double(move) ? 1 : 0,
			get_move_enpassant(move) ? 1 : 0,
			get_move_castling(move) ? 1 : 0);
	}
	// print total number of moves
	printf("\nTotal number of moves: %d\n\n", move_list->count);
}

void test_move_encoding()
{
	int move = encode_move(e2, e4, K, Q, 1, 1, 1, 1);

	printf("src square:%s\n", square_to_coordinates[get_move_source(move)]);
	printf("target square:%s\n", square_to_coordinates[get_move_target(move)]);
	printf("Piece:%c\n", ascii_pieces[get_move_piece(move)]);
	printf("Promoted:%c\n", ascii_pieces[get_move_promoted(move)]);

	printf("Capture:%s\n", (get_move_capture(move) ? "1" : "0"));
	printf("double:%s\n", (get_move_double(move) ? "1" : "0"));
	printf("enpassant:%s\n", (get_move_enpassant(move) ? "1" : "0"));
	printf("Castle:%s\n", (get_move_castling(move) ? "1" : "0"));
}


#pragma endregion
/****************************************\
 ========================================
			  Leaper attacks
 ========================================
\****************************************/
#pragma region
//generate not a file, not h file and so on
void calc_mask_files()
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
U64 mask_pawn_attacks(bool side, int square)
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
U64 mask_knight_attacks(int square)
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
U64 mask_king_attacks(int square)
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
U64 mask_bishop_attacks(int square)
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
U64 mask_rook_attacks(int square)
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
U64 bishop_blocker_attacks(int square, U64 block)
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
U64 rook_blocker_attacks(int square, U64 block)
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
/****************************************\
 ========================================
			 Initialization
 ========================================
\****************************************/
#pragma region
void init_leapers_attacks()
{
	for (int square = 0; square < SQUARE_NB; ++square)
	{
		//Pawn attacks
		pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
		pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);

		//Knight attacks
		knight_attacks[square] = mask_knight_attacks(square);

		//King attacks
		king_attacks[square] = mask_king_attacks(square);
	}
}

void init_all()
{
	init_leapers_attacks();
	init_sliders_attacks(BISHOP);
	init_sliders_attacks(ROOK);
}
#pragma endregion
/****************************************\
 ========================================
				 Random
 ========================================
\****************************************/
#pragma region
//generate 32bit psuedo legal moves
static inline unsigned int randu32()
{
	//fast method
	//seed = (214013U * seed + 2531011U);
	//return (seed >> 16) & 0x7FFFU;

	random_state ^= random_state << 13;
	random_state ^= random_state >> 17;
	random_state ^= random_state << 5;

	return random_state;
}
static inline U64 randu64()
{
	U64 n1, n2, n3, n4;

	//only keep top 16 bits
	n1 = (U64)randu32() & 0xFFFF;
	n2 = (U64)randu32() & 0xFFFF;
	n3 = (U64)randu32() & 0xFFFF;
	n4 = (U64)randu32() & 0xFFFF;

	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}
U64 generate_magic_number()
{
	return randu64() & randu64() & randu64();
}
#pragma endregion
/****************************************\
 ========================================
				Magics
 ========================================
\****************************************/
#pragma region Magics
// set occupancies
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
	// occupancy map
	U64 occupancy = 0ULL;

	// loop over the range of bits within attack mask
	for (int count = 0; count < bits_in_mask; count++)
	{
		// get 1st bit index of attacks mask
		int square = get_lsb_index(attack_mask);

		// pop 1st bit in attack map
		pop_bit(attack_mask, square);

		// make sure occupancy is on board
		if (index & (1 << count))
			// populate occupancy map
			occupancy |= (1ULL << square);
	}
	// return occupancy map
	return occupancy;
}

//find magic number
U64 find_magic_number(int square, int relevant_bits, int bishop)
{
	const int size = 4096;

	U64 occupancies[size], attacks[size], used_attacks[size];
	U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

	//init occupancy indicies
	int occupancy_indices = 1 << relevant_bits;

	for (int i = 0; i < occupancy_indices; i++)
	{
		occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);

		attacks[i] = bishop ? bishop_blocker_attacks(square, occupancies[i]) :
			rook_blocker_attacks(square, occupancies[i]);
	}

	for (int random_cnt = 0; random_cnt < 100000000; random_cnt++)
	{
		//generate magic number candidate
		U64 magic_number = generate_magic_number();

		//skip bad numbers
		if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

		//init used attacks
		memset(used_attacks, 0ULL, sizeof(used_attacks));

		int i, fail;
		for (i = 0, fail = 0; !fail && i < occupancy_indices; i++)
		{
			//init magic index
			int magic_index = (int)((occupancies[i] * magic_number) >> (64 - relevant_bits));

			//if magic index works
			if (used_attacks[magic_index] == 0ULL)
			{
				//init used attacks
				used_attacks[magic_index] = attacks[i];
			}
			else if (used_attacks[magic_index] != attacks[i])
			{
				//magic index doesnt work
				fail = 1;
			}
		}
		if (!fail)
			return magic_number;
	}
	//doesnt work
	printf(" Magic number fails!\n");
	return 0ULL;
}


void init_magic_numbers()
{
	throw std::string("magic numbers not defined");

	/* for (int square = 0; square < SQUARE_NB; square++)
		 rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);
	 for (int square = 0; square < SQUARE_NB; square++)
		 bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);*/
}

void init_sliders_attacks(int bishop)
{
	for (int square = 0; square < 64; square++)
	{
		//init bishop & rook masks
		bishop_masks[square] = mask_bishop_attacks(square);
		rook_masks[square] = mask_rook_attacks(square);

		//init current mask
		U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

		//init occupancy bit count
		int relevant_bits_count = count_bits(attack_mask);

		//init occupancy indicies
		int occupancy_indicies = (1 << relevant_bits_count);

		//loop over occupancy
		for (int i = 0; i < occupancy_indicies; i++)
		{
			if (bishop == BISHOP)
			{
				//init current occupoancy
				U64 occupancy = set_occupancy(i, relevant_bits_count, attack_mask);
				//init magic index
				int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
				//init bishop attacks
				bishop_attacks[square][magic_index] = bishop_blocker_attacks(square, occupancy);
			}
			else//rook
			{
				//init current occupoancy
				U64 occupancy = set_occupancy(i, relevant_bits_count, attack_mask);
				//init magic index
				int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
				//init rook attacks				
				rook_attacks[square][magic_index] = rook_blocker_attacks(square, occupancy);
			}
		}
	}
}

//on the fly
static inline U64 get_bishop_attacks(int square, U64 occupancy)
{
	//get bishop attacks assuming current board occupancy
	occupancy &= bishop_masks[square];
	occupancy *= bishop_magic_numbers[square];
	occupancy >>= 64 - bishop_relevant_bits[square];

	return bishop_attacks[square][occupancy];
}
//on the fly
static inline U64 get_rook_attacks(int square, U64 occupancy)
{
	//get rook attacks assuming current board occupancy
	occupancy &= rook_masks[square];
	occupancy *= rook_magic_numbers[square];
	occupancy >>= 64 - rook_relevant_bits[square];

	//print_bitboard(occupancy);

	return rook_attacks[square][occupancy];
}
//on the fly
static inline U64 get_queen_attacks(int square, U64 occupancy)
{
	return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}


#pragma endregion
/****************************************\
 ========================================
			Move Generator
 ========================================
\****************************************/
#pragma region Move Generator

enum {ALL_MOVES, ONLY_CAPTURES};

//version 1


#define copy_board()                                                 \
U64 bitboards_copy[12], occupancies_copy[3];                         \
int enpassant_copy, castle_copy;                                     \
memcpy(bitboards_copy, bitboards, 96);                               \
memcpy(occupancies_copy, occupancies, 24);						     \
enpassant_copy = enpassant, castle_copy = castle; 


#define take_back()                                                  \
memcpy(bitboards, bitboards_copy, 96);                               \
memcpy(occupancies, occupancies_copy, 24);                           \
side ^= 1, enpassant = enpassant_copy, castle = castle_copy; 





//version 2

//96 + 24 + 4 + 4 = 128
//=> 128 * depth

//char* boardstates = new char[128*60];

//long index = 0;

//static inline void copy_board()
//{		
//	memcpy(&(boardstates[index]), bitboards, 96);
//	index += 96;
//	memcpy(&(boardstates[index]), occupancies, 24);
//	index += 24;
//	memcpy(&(boardstates[index]), &enpassant, 4);
//	index += 4;
//	memcpy(&(boardstates[index]), &castle, 4);	
//	index += 4;
//
//	//memcpy(&(boardstates[index]), &side, 4);
//	//index += 4;
//}



//static inline void take_back()
//{	
//	index -= 4;	
//	castle = boardstates[index];
//	index -= 4;
//	
//	enpassant = boardstates[index];
//	index -= 24;
//
//	memcpy(occupancies, &(boardstates[index]), 24);	
//	index -= 96;
//	memcpy(bitboards, &(boardstates[index]), 96);
//
//	side ^= 1;
//}



//static inline void copy_board()
//{
//	memcpy(boardstates, bitboards, 96);
//	boardstates += 96;
//	
//	memcpy(boardstates, occupancies, 24);
//	boardstates += 24;
//	
//	//*boardstates = enpassant;
//	memcpy(boardstates, &enpassant, 4);
//	boardstates += 4;
//	
//	//*boardstates = castle;
//	memcpy(boardstates, &castle, 4);
//	boardstates += 4;
//		
//}
//
//static inline void take_back()
//{	
//	boardstates -= 4;
//	castle = *boardstates;
//	
//	boardstates -= 4;
//	enpassant = *boardstates;
//	
//	boardstates -= 24;
//	memcpy(occupancies, boardstates, 24);
//	
//	boardstates -= 96;
//	memcpy(bitboards, boardstates, 96);
//
//	side ^= 1;
//}






//#define copy_board()\
//	memcpy(boardstates, bitboards, 96);\
//	boardstates += 96;\
//	\
//	memcpy(boardstates, occupancies, 24);\
//	boardstates += 24;\
//	\
//	int enpassant_copy = enpassant, castle_copy = castle;
//
//
//
//#define take_back()\
//	castle = castle_copy, enpassant = enpassant_copy;\
//	\
//	boardstates -= 24;\
//	memcpy(occupancies, boardstates, 24);\
//	\
//	boardstates -= 96;\
//	memcpy(bitboards, boardstates, 96);\
//	\
//	side ^= 1;





//is given square attacked by given side?
//color = attacking color
static inline bool is_square_attacked(int square, int side)
{
	//Slider attacks (Queen, bishop, rook)

	//Bishop attacks
	if (get_bishop_attacks(square, occupancies[BOTH]) & ((side == WHITE) ? bitboards[B] : bitboards[b])) return true;
	//Rook attacks
	if (get_rook_attacks(square, occupancies[BOTH]) & ((side == WHITE) ? bitboards[R] : bitboards[r])) return true;
	//Queen attacks
	if (get_queen_attacks(square, occupancies[BOTH]) & ((side == WHITE) ? bitboards[Q] : bitboards[q])) return true;


	//leaper attacks (pawn, knight, king)

	//attacked by white pawn
	if ((side == WHITE) && (pawn_attacks[BLACK][square] & bitboards[P])) return true;
	//attacked by black pawn
	if ((side == BLACK) && (pawn_attacks[WHITE][square] & bitboards[p])) return true;
	//knight attacks
	if (knight_attacks[square] & ((side == WHITE) ? bitboards[N] : bitboards[n])) return true;
	//king attacks
	if (king_attacks[square] & ((side == WHITE) ? bitboards[K] : bitboards[k])) return true;

	
	return false;
}

static inline void add_move(Moves* move_list, int move)
{
	//store move
	move_list->moves[move_list->count] = move;

	move_list->count++;
}

//generates all moves
static inline void generate_moves(Moves* move_list)
{
	//set number of moves to 0
	move_list->count = 0;

	int source_square, target_square;

	U64 bitboard, attacks;

	//loop over bitboards
	for (int piece = P; piece <= k; piece++)
	{
		bitboard = bitboards[piece];

		//generate white pawn moves and castling moves
		if (side == WHITE)
		{
			if (piece == P)//white pawn
			{
				//While there are white pawns left
				while (bitboard)
				{
					//get the first pawn
					source_square = get_lsb_index(bitboard);

					//move up one row
					target_square = source_square - 8;

					//generate quite pawn moves. if onboard and no capture (single pawn push)
					if (!(target_square < a8) && !get_bit(occupancies[BOTH], target_square))
					{
						//pawn promotion
						if (source_square >= a7 && source_square <= h7)
						{
							//add moves here
							//printf("w pawn promotion: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));


						}
						else
						{
							//single pawn push
							//printf("w pawn push: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

							//double pawn push
							if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[BOTH], target_square - 8))
							{
								//printf("w double pawn push: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square - 8]);
								add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));

							}
						}

					}


					//init pawn attacks
					attacks = pawn_attacks[WHITE][source_square] & occupancies[BLACK];

					//generate captures
					while (attacks)
					{
						target_square = get_lsb_index(attacks);

						//pawn promotion
						if (source_square >= a7 && source_square <= h7)
						{
							//add moves here
							//printf("w pawn promotion capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
						}
						//normal capture
						else
						{
							//printf("w pawn capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
						}
						//remove attack
						pop_bit(attacks, target_square);
					}
					//generate en passant captures
					if (enpassant != NO_SQ)
					{
						//get attacks and bitwise and with enpassant square (bit)
						U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

						//check if available
						if (enpassant_attacks)
						{
							//en passant target square
							int target_enpassant = get_lsb_index(enpassant_attacks);

							//printf("w pawn enpassant capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_anpassant]);
							add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));		
						}

					}

					//pop lsb - remove piece from temp bitboard
					pop_bit(bitboard, source_square);
				}
			}



			//white castling moves
			if (piece == K)
			{
				//king side castling
				if (castle & WK)
				{
					//check if squarees are empty
					if (!get_bit(occupancies[BOTH], f1) && !get_bit(occupancies[BOTH], g1))
					{
						//make sure king and next square not attacked, rest of aquare handled by legal move function
						if (!is_square_attacked(e1, BLACK) && !is_square_attacked(f1, BLACK))
						{
							//printf("castle WK: e1g1\n");
							add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
						}
					}
				}
				//queen side castling
				if (castle & WQ)
				{
					//check if squarees are empty
					if (!get_bit(occupancies[BOTH], d1) && !get_bit(occupancies[BOTH], c1) && !get_bit(occupancies[BOTH], b1))
					{
						//make sure king and next square not attacked, rest of aquare handled by legal move function
						if (!is_square_attacked(e1, BLACK) && !is_square_attacked(d1, BLACK))
						{
							//printf("castle WQ: e1c1\n");
							add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));

						}
					}
				}

			}



		}
		//generate black pawn moves and castling moves
		else
		{
			if (piece == p)//black pawn
			{
				//While there are black pawns left
				while (bitboard)
				{
					//get the first pawn
					source_square = get_lsb_index(bitboard);

					//move down one row
					target_square = source_square + 8;

					//generate quite pawn moves. if onboard and no capture (single pawn push)
					if (!(target_square > h1) && !get_bit(occupancies[BOTH], target_square))
					{
						//pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
							//add move here
							//printf("b pawn promotion: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);

							add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
						}
						else
						{
							//single pawn push
							//printf("b pawn push: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

							//double pawn push
							if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[BOTH], target_square + 8))
							{

								//printf("b double pawn push: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square + 8]);
								add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
							}
						}

					}

					//init black pawn attacks
					attacks = pawn_attacks[BLACK][source_square] & occupancies[WHITE];

					//generate captures
					while (attacks)
					{
						target_square = get_lsb_index(attacks);

						//pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
							//add moves here
							//printf("b pawn promotion capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
							add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
							add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
						}
						//normal capture
						else
						{
							//printf("b pawn capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);

							add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
						}
						//remove attack
						pop_bit(attacks, target_square);
					}
					//generate en passant captures
					if (enpassant != NO_SQ)
					{
						//get attacks and bitwise and with enpassant square (bit)
						U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);

						//check if available
						if (enpassant_attacks)
						{
							//en passant target square
							int target_enpassant = get_lsb_index(enpassant_attacks);

							//printf("b pawn enpassant capture: %s%s\n", square_to_coordinates[source_square], square_to_coordinates[target_enpassant]);

							add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
						}
					}


					//pop lsb - remove piece from temp bitboard
					pop_bit(bitboard, source_square);
				}
			}

			//black castling moves			
			if (piece == k)
			{
				//king side castling
				if (castle & BK)
				{
					//check if squarees are empty
					if (!get_bit(occupancies[BOTH], f8) && !get_bit(occupancies[BOTH], g8))
					{
						//make sure king and next square not attacked, rest of aquare handled by legal move function
						if (!is_square_attacked(e8, WHITE) && !is_square_attacked(f8, WHITE))
						{
							//printf("castle BK: e8g8\n");

							add_move(move_list, encode_move(e8, g8,piece,0,0,0,0,1));
						}
					}
				}
				//queen side castling
				if (castle & BQ)
				{
					//check if squarees are empty
					if (!get_bit(occupancies[BOTH], d8) && !get_bit(occupancies[BOTH], c8) && !get_bit(occupancies[BOTH], b8))
					{
						//make sure king and next square not attacked, rest of aquare handled by legal move function
						if (!is_square_attacked(e8, WHITE) && !is_square_attacked(d8, WHITE))
						{
							//printf("castle BQ: e8c8\n");

							add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
						}
					}
				}
			}
		}

		//generate knight moves
		if ((side == WHITE) ? piece == N : piece == n)//piece equal to current side
		{
			//loop over all the pieces
			while (bitboard)
			{
				source_square = get_lsb_index(bitboard);

				//current piece attacks to get target                   //not capture white pieces //not capture black pieces
				attacks = knight_attacks[source_square] & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

				//loop over all attacks
				while (attacks)
				{
					target_square = get_lsb_index(attacks);
						
					//quite move if no piece is captured
					if (!get_bit(((side == WHITE) ? occupancies[BLACK] : occupancies[WHITE]), target_square))
					{
						//printf(" %s%s  knight quite\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
						
					}
					//capture move
					else
					{						 					
					     //printf(" %s%s  knight capture\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
					}
					
					pop_bit(attacks, target_square);
				}

				pop_bit(bitboard, source_square);
			}
		}

		//generate bishop moves
		if ((side == WHITE) ? piece == B : piece == b)//piece equal to current side
		{
			//loop over all the pieces
			while (bitboard)
			{
				source_square = get_lsb_index(bitboard);

				//current piece attacks to get target                   //not capture white pieces //not capture black pieces
				attacks = get_bishop_attacks(source_square,occupancies[BOTH]) & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

				//loop over all attacks
				while (attacks)
				{
					target_square = get_lsb_index(attacks);

					//quite move if no piece is captured
					if (!get_bit(((side == WHITE) ? occupancies[BLACK] : occupancies[WHITE]), target_square))
					{
						//printf(" %s%s  bishop quite\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
					}
					else
					{
						//capture move					
						//printf(" %s%s  bishop capture\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
					}

					pop_bit(attacks, target_square);
				}

				pop_bit(bitboard, source_square);
			}
		}

		//generate rook moves
		if ((side == WHITE) ? piece == R : piece == r)//piece equal to current side
		{
			//loop over all the pieces
			while (bitboard)
			{
				source_square = get_lsb_index(bitboard);

				//current piece attacks to get target                   //not capture white pieces //not capture black pieces
				attacks = get_rook_attacks(source_square, occupancies[BOTH]) & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

				//loop over all attacks
				while (attacks)
				{
					target_square = get_lsb_index(attacks);

					//quite move if no piece is captured
					if (!get_bit(((side == WHITE) ? occupancies[BLACK] : occupancies[WHITE]), target_square))
					{
						//printf(" %s%s  rook quite\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
					}
					else
					{
						//capture move					
						//printf(" %s%s  rook capture\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
					}

					pop_bit(attacks, target_square);
				}

				pop_bit(bitboard, source_square);
			}
		}

		//generate queen moves
		if ((side == WHITE) ? piece == Q : piece == q)//piece equal to current side
		{
			//loop over all the pieces
			while (bitboard)
			{
				source_square = get_lsb_index(bitboard);

				//current piece attacks to get target                   //not capture white pieces //not capture black pieces
				attacks = get_queen_attacks(source_square, occupancies[BOTH]) & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

				//loop over all attacks
				while (attacks)
				{
					target_square = get_lsb_index(attacks);

					//quite move if no piece is captured
					if (!get_bit(((side == WHITE) ? occupancies[BLACK] : occupancies[WHITE]), target_square))
					{
						//printf(" %s%s  queen quite\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
					}
					else
					{
						//capture move					
						//printf(" %s%s  queen capture\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
					}

					pop_bit(attacks, target_square);
				}

				pop_bit(bitboard, source_square);
			}
		}

		//generate king moves
		if ((side == WHITE) ? piece == K : piece == k)//piece equal to current side
		{
			//loop over all the pieces
			while (bitboard)
			{
				source_square = get_lsb_index(bitboard);

				//current piece attacks to get target                   //not capture white pieces //not capture black pieces
				attacks = king_attacks[source_square] & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

				//loop over all attacks
				while (attacks)
				{
					target_square = get_lsb_index(attacks);

					//quite move if no piece is captured
					if (!get_bit(((side == WHITE) ? occupancies[BLACK] : occupancies[WHITE]), target_square))
					{
						//printf(" %s%s  king quite\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
					}
					else
					{
						//capture move					
						//printf(" %s%s  king capture\n", square_to_coordinates[source_square], square_to_coordinates[target_square]);
						add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
					}

					pop_bit(attacks, target_square);
				}

				pop_bit(bitboard, source_square);
			}
		}
	}
}

static inline int make_move(int move, int move_flag)
{
	//only make move if move_flag == all_moves or if it is a capture move
	if (move_flag == ALL_MOVES || get_move_capture(move))
	{
		//copy_board();

		//parse move
		int m_source_square = get_move_source(move);
		int m_target_square = get_move_target(move);
		int m_capture = get_move_capture(move);
		int m_piece = get_move_piece(move);
		int m_promoted_piece = get_move_promoted(move);
		
		int m_double_push = get_move_double(move);
		int m_enpassant = get_move_enpassant(move);
		int m_castling = get_move_castling(move);

		//remove piece
		pop_bit(bitboards[m_piece], m_source_square);
		//set piece
		set_bit(bitboards[m_piece], m_target_square);

		//now after the move is made do all this

		//captured piece needs to be removed
		if (m_capture)
		{
			int start_piece, end_piece;

			//white to move
			if (side == WHITE)
			{
				start_piece = p;
				end_piece = k;
			}
			//black to move
			else
			{
				start_piece = P;
				end_piece = K;
			}

			//loop only over oppsite side pieces
			for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
			{
				//if piece on target square remove it
				if (get_bit(bitboards[bb_piece], m_target_square))
				{
					pop_bit(bitboards[bb_piece], m_target_square);
					break;
				}
			}
		}

		//pawn promotions
		if (m_promoted_piece)
		{
			//remove pawn
			if (side == WHITE)
			{
				pop_bit(bitboards[P], m_target_square);
			}
			else
			{
				pop_bit(bitboards[p], m_target_square);
			}

			// set promoted piece on chess board
			set_bit(bitboards[m_promoted_piece], m_target_square);	
		}

		if (m_enpassant)
		{			
			// white to move
			if (side == WHITE)
			{
				// remove captured pawn
				pop_bit(bitboards[p], m_target_square + 8);

				// remove pawn from hash key
				//hash_key ^= piece_keys[p][target_square + 8];
			}			
			else
			{
				// remove captured pawn
				pop_bit(bitboards[P], m_target_square - 8);

				// remove pawn from hash key
				//hash_key ^= piece_keys[P][target_square - 8];
			}
		}

		//always reset enpassant
		enpassant = NO_SQ;

		//for enpassant
		if (m_double_push)
		{
			// white to move
			if (side == WHITE)
			{				
				enpassant = m_target_square + 8;

				// hash enpassant
				//hash_key ^= enpassant_keys[target_square + 8];
			}

			// black to move
			else
			{			
				enpassant = m_target_square - 8;

				// hash enpassant
				//hash_key ^= enpassant_keys[target_square - 8];
			}
		}

		if (m_castling)
		{
			switch (m_target_square)
			{
				//WK
				case g1:
					pop_bit(bitboards[R], h1);//remove rook
					set_bit(bitboards[R], f1);//set rook				
					break;
			    //WQ
				case c1:
					pop_bit(bitboards[R], a1);//remove rook
					set_bit(bitboards[R], d1);//set rook	
					break;
				//BK
				case g8:
					pop_bit(bitboards[r], h8);//remove rook
					set_bit(bitboards[r], f8);//set rook		
					break;
				//BQ
				case c8:
					pop_bit(bitboards[r], a8);//remove rook
					set_bit(bitboards[r], d8);//set rook	
					break;
			}
		}

		//update castling rights
		castle &= castling_rights[m_source_square];
		castle &= castling_rights[m_target_square];
		
		//update occupancies
		memset(occupancies, 0ULL, 24);
		//white pieces
		for (int i = P; i <= K; i++)	
			occupancies[WHITE] |= bitboards[i];
		//black pieces
		for (int i = p; i <= k; i++)
			occupancies[BLACK] |= bitboards[i];
		//both
		occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];


		//change side
		side ^= 1;


		// make sure that king has not been exposed into a check
		if (is_square_attacked((side == WHITE) ? get_lsb_index(bitboards[k]) : get_lsb_index(bitboards[K]), side))
		{
			// take move back
			//take_back();

			
			// return illegal move
			return 0;
		}	
		else
			// return legal move
			return 1;

	}
	//dont make move
	else		
		return 0;					
}


#pragma endregion
/****************************************\
 ========================================
				 Perft
 ========================================
\****************************************/
#pragma region


//leaf nodes (number of positions reached at given depth
U64 nodes = 0ULL;

static inline void perft_driver(int depth)
{
	//escape
	if (depth <= 0)
	{
		nodes++;
		return;
	}

	Moves move_list[1];
	generate_moves(move_list);

	for (int move_count = 0; move_count < move_list->count; move_count++)
	{
		// preserve board state
		copy_board();
		

		// make move
		if (!make_move(move_list->moves[move_count], ALL_MOVES))
		{
			take_back();
			continue;
		}

		// call perft driver recursively
		perft_driver(depth - 1);

		// take back
		take_back();	
	}
}

void perft_test(int depth)
{	
	auto startTime = std::chrono::high_resolution_clock::now();

	Moves move_list[1];
	generate_moves(move_list);

	if (depth > 0)
	{
		for (int move_count = 0; move_count < move_list->count; move_count++)
		{
			// preserve board state
			copy_board();			

			auto move = move_list->moves[move_count];

			// make move
			if (!make_move(move, ALL_MOVES)) 
			{
				take_back();
				continue;
			}
			

			U64 cumulative_nodes = nodes;

			// call perft driver recursively

			perft_driver(depth - 1);

			U64 current_nodes = nodes - cumulative_nodes;

			// take back
			take_back();			
			
			printf("%s%s%c  nodes: %llu\n",
				square_to_coordinates[get_move_source(move)],
				square_to_coordinates[get_move_target(move)],
				promoted_pieces[get_move_promoted(move)],
				current_nodes
			);
		}
	}
	else nodes++;

	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	auto elapsedMS = ((long double)elapsedMicro) / 1000;
	auto elapsedS = elapsedMS / 1000;

	printf("\nDepth:%d\n", depth);
	printf("Nodes:%llu\n", nodes);
	printf("Total Time:%.2LF (MS)\n", elapsedMS);
	//printf("Seconds:%.2LF (S)\n", elapsedS);
	printf("Nodes per second:%.0LF\n\n", (nodes / elapsedS));
}

#pragma endregion
/****************************************\
 ========================================
				My Search
 ========================================
\****************************************/
#pragma region

const int piece_scores[12] = { 10, 30, 31, 50, 90, 2000, -10, -30, -31, -50, -90, -2000 };


static inline int Mposition_evaluate(int cnt,int depth)
{

	/*if (bitboards[K] == 0 || bitboards[k] == 0)
	{
		print_board();
		printf("Errr king missing");
		std::cin.get();
		assert(0 && "King is missing");
	}*/

	/*if (bitboards[K] == 0)
		return 30000;
	if (bitboards[k] == 0)
		return -30000;*/
	


	

	if (cnt == 0)
	{	
		//printf("count was zero!!!!\n");

		
		if (is_square_attacked(get_lsb_index(bitboards[K]), BLACK))//white king attacked by black
		{
			//printf("white is mate\n");

			if (depth <= 0)
				return -2000;

			return -2000 * depth;
		}
			
		if (is_square_attacked(get_lsb_index(bitboards[k]), WHITE))//black king attacked by white
		{
			//printf("black is mate\n");

			if (depth <= 0)
				return 2000;

			return 2000 * depth;
		}

		//printf("stalemate\n");
		return 0;	

	}
	

	int score = 0;


	for (int i = P; i <= k; i++)
		score += ((count_bits(bitboards[i]) + depth) * piece_scores[i]);
	//printf("score:%d\n",score);
	return score;
}


static inline int MMiniMax(int depth, bool Maximizing)
{
	Moves moves[1];
	generate_moves(moves);
	
	int cnt = 0;
	for (int i = 0; i < moves->count; i++)
	{
		copy_board();
		if (!make_move(moves->moves[i], ALL_MOVES))
		{
			take_back();
			continue;
		}
		take_back();

		//moves->legal[i] = moves->moves[i];

		//legal[cnt++] = moves->moves[i];	
		cnt++;
	}

	if (depth == 0 || cnt == 0)
		return Mposition_evaluate(cnt,depth);

	if (Maximizing)//white
	{
		int maxeval = INT_MIN;
					
		for (int i = 0; i < moves->count; i++)
		{		
			copy_board();

			if (!make_move(moves->moves[i], ALL_MOVES))
			{
				take_back();
				continue;
			}			


			int eval = MMiniMax(depth - 1, false);

			take_back();


			maxeval = std::max(maxeval, eval);		
		}
		return maxeval;
	}
	else//black
	{
		int mineval = INT_MAX;
				
		for (int i = 0; i < moves->count; i++)
		{			
			copy_board();

			if (!make_move(moves->moves[i], ALL_MOVES))
			{
				take_back();
				continue;
			}			

			int eval = MMiniMax(depth - 1, true);

			take_back();
		

			mineval = std::min(mineval, eval);		
		}	
		return mineval;
	}
}


////not working
//int legal[255];
//
//static inline int MiniMaxV2(int depth, bool Maximizing)
//{
//	Moves moves[1];
//	generate_moves(moves);
//
//	int cnt = 0;
//	for (int i = 0; i < moves->count; i++)
//	{
//		copy_board();
//		if (!make_move(moves->moves[i], ALL_MOVES))
//		{
//			take_back();
//			continue;
//		}
//		take_back();
//
//		legal[cnt] = moves->moves[i];
//		cnt++;
//	}
//
//	if (depth == 0 || cnt == 0)
//		return position_evaluate(cnt,depth);
//
//	if (Maximizing)//white
//	{
//		int maxeval = INT_MIN;
//
//		for (int i = 0; i < cnt; i++)
//		{
//			copy_board();
//			
//			make_move(legal[i], ALL_MOVES);
//
//			int eval = MiniMaxV2(depth - 1, false);
//
//			take_back();
//
//
//			maxeval = std::max(maxeval, eval);
//		}
//		return maxeval;
//	}
//	else//black
//	{
//		int mineval = INT_MAX;
//
//		for (int i = 0; i < cnt; i++)
//		{
//			copy_board();
//		
//			make_move(legal[i], ALL_MOVES);
//
//			int eval = MiniMaxV2(depth - 1, true);
//
//			take_back();
//
//
//			mineval = std::min(mineval, eval);
//		}
//		return mineval;
//	}
//}


int Mget_best_move(int depth)
{
	Moves moves[1];
	generate_moves(moves);

	std::vector<int> bestmoves;

	printf("\nCount: %d\n", (moves->count));
	
	if (side == WHITE)
	{
		int bestScore = INT_MIN;

		for (int i = 0; i < moves->count; i++)//paralize this
		{
			copy_board();

			if (!make_move(moves->moves[i], ALL_MOVES))
			{
				take_back();
				continue;
			}
			
			int eval = MMiniMax(depth - 1, false);
			printf("no%d  score: %d  move:",(i+1), eval);
			print_move(moves->moves[i]);
			printf("\n");

			take_back();

			
			if (eval >= bestScore)
			{
				if (eval == bestScore)
					bestmoves.push_back(moves->moves[i]);
				else
				{
					printf("White new eval:%d\n", eval);
					bestmoves.clear();
					bestmoves.push_back(moves->moves[i]);
					bestScore = eval;
				}													
			}
		}					
	}
	else
	{
		int bestScore = INT_MAX;

		for (int i = 0; i < moves->count; i++)//paralize this
		{		
			copy_board();

			if (!make_move(moves->moves[i], ALL_MOVES))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, true);
			printf("no%d  score: %d  move:", (i + 1), eval);
			print_move(moves->moves[i]);
			printf("\n");



			take_back();

			if (eval <= bestScore)
			{
				if (eval == bestScore)
					bestmoves.push_back(moves->moves[i]);
				else
				{
					printf("black new eval:%d\n", eval);

					bestmoves.clear();
					bestmoves.push_back(moves->moves[i]);
					bestScore = eval;
				}		
			}
		}		
	}
	//printf("size: %d\n", bestmoves.size());

	//printf("rand index: %d\n",(rand() % bestmoves.size()));

	return bestmoves[rand() % bestmoves.size()];
}

void Msearch_position(int depth)
{
	printf("depth:%d\n", depth);
	//printf("bestmove d2d4\n");

	int best = Mget_best_move(depth);

	printf("bestmove ");
	print_move(best);
}

#pragma endregion
/****************************************\
 ========================================
				Evaluation
 ========================================
\****************************************/
#pragma region
// material scrore

/*
	♙ =   100   = ♙
	♘ =   300   = ♙ * 3
	♗ =   350   = ♙ * 3 + ♙ * 0.5
	♖ =   500   = ♙ * 5
	♕ =   1000  = ♙ * 10
	♔ =   10000 = ♙ * 100

*/

const int material_score[12] = 
{
	100,      // white pawn score
	300,      // white knight scrore
	350,      // white bishop score
	500,      // white rook score
   1000,      // white queen score
  10000,      // white king score
   -100,      // black pawn score
   -300,      // black knight scrore
   -350,      // black bishop score
   -500,      // black rook score
  -1000,      // black queen score
 -10000,      // black king score
};

// pawn positional score
const int pawn_score[64] =
{
	90,  90,  90,  90,  90,  90,  90,  90,
	30,  30,  30,  40,  40,  30,  30,  30,
	20,  20,  20,  30,  30,  30,  20,  20,
	10,  10,  10,  20,  20,  10,  10,  10,
	 5,   5,  10,  20,  20,   5,   5,   5,
	 0,   0,   0,   5,   5,   0,   0,   0,
	 0,   0,   0, -10, -10,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] =
{
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5,   0,   0,  10,  10,   0,   0,  -5,
	-5,   5,  20,  20,  20,  20,   5,  -5,
	-5,  10,  20,  30,  30,  20,  10,  -5,
	-5,  10,  20,  30,  30,  20,  10,  -5,
	-5,   5,  20,  10,  10,  20,   5,  -5,
	-5,   0,   0,   0,   0,   0,   0,  -5,
	-5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[64] =
{
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,  20,   0,  10,  10,   0,  20,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,  10,   0,   0,   0,   0,  10,   0,
	 0,  30,   0,   0,   0,   0,  30,   0,
	 0,   0, -10,   0,   0, -10,   0,   0
};

// rook positional score
const int rook_score[64] =
{
	50,  50,  50,  50,  50,  50,  50,  50,
	50,  50,  50,  50,  50,  50,  50,  50,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,  10,  20,  20,  10,   0,   0,
	 0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] =
{
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   0,   5,   5,   5,   5,   0,   0,
	 0,   5,   5,  10,  10,   5,   5,   0,
	 0,   5,  10,  20,  20,  10,   5,   0,
	 0,   5,  10,  20,  20,  10,   5,   0,
	 0,   0,   5,  10,  10,   5,   0,   0,
	 0,   5,   5,  -5,  -5,   0,   5,   0,
	 0,   0,   5,   0, -15,   0,  10,   0
};

// mirror positional score tables for opposite side
const int mirror_score[128] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

/*
		  Rank mask            File mask           Isolated mask        Passed pawn mask
		for square a6        for square f2         for square g2          for square c4
	8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 1 0 0    8  0 0 0 0 0 1 0 1     8  0 1 1 1 0 0 0 0
	7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 1 0 0    7  0 0 0 0 0 1 0 1     7  0 1 1 1 0 0 0 0
	6  1 1 1 1 1 1 1 1    6  0 0 0 0 0 1 0 0    6  0 0 0 0 0 1 0 1     6  0 1 1 1 0 0 0 0
	5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 1 0 0    5  0 0 0 0 0 1 0 1     5  0 1 1 1 0 0 0 0
	4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 1 0 0    4  0 0 0 0 0 1 0 1     4  0 0 0 0 0 0 0 0
	3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 1 0 0    3  0 0 0 0 0 1 0 1     3  0 0 0 0 0 0 0 0
	2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 1 0 0    2  0 0 0 0 0 1 0 1     2  0 0 0 0 0 0 0 0
	1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 1 0 0    1  0 0 0 0 0 1 0 1     1  0 0 0 0 0 0 0 0
	   a b c d e f g h       a b c d e f g h       a b c d e f g h        a b c d e f g h
*/


static inline int evaluate()
{
	int score = 0;

	//current pieces bitboard copy
	U64 bitboard;

	//init piece and score
	int piece, square;


	for (int bb_piece = P; bb_piece <= k; bb_piece++)
	{
		bitboard = bitboards[bb_piece];

		while (bitboard)
		{
			piece = bb_piece;
			square = get_lsb_index(bitboard);

			//material score
			score += material_score[piece];

			//positional score
			switch (piece)
			{
				//white
				case P: score += pawn_score[square]; break;
				case N: score += knight_score[square]; break;
				case B: score += bishop_score[square]; break;
				case R: score += rook_score[square]; break;			
				case K: score += king_score[square]; break;
				//black
				case p: score -= pawn_score[mirror_score[square]]; break;
				case n: score -= knight_score[mirror_score[square]]; break;
				case b: score -= bishop_score[mirror_score[square]]; break;
				case r: score -= rook_score[mirror_score[square]]; break;
				case k: score -= king_score[mirror_score[square]]; break;

				default: break;
			}


			//pop lsb bit
			pop_bit(bitboard, square);
		}
	}


	return (side == WHITE) ? score : -score;
}


#pragma endregion
/****************************************\
 ========================================
				Search
 ========================================
\****************************************/
#pragma region

//alpha = maximizing
//beta = minimizing

//half move counter
int ply;

//best move
int best_move;

static inline int negamax(int alpha, int beta, int depth)
{
	if (depth == 0)	
		return evaluate();
	
	//number of nodes
	nodes++;

	int current_best = 0;
	int old_alpha = alpha;

	Moves moves[1];
	generate_moves(moves);


	for (int i = 0; i < moves->count; i++)
	{
		copy_board();

		ply++;

		if (!make_move(moves->moves[i], ALL_MOVES))
		{
			take_back();
			ply--;
			continue;
		}
		
		int score = -negamax(-beta, -alpha, depth - 1);

		ply--;
		take_back();

		//fails high
		if (score >= beta) return beta;

		if (score > alpha)
		{
			//PV node (Prinsipal variation)
			alpha = score;

			//root move
			if (ply == 0)
			{
				current_best = moves->moves[i];
			}
		}


	}


	if (old_alpha != alpha)//best move
		best_move = current_best;

	//fails low
	return alpha;

}

void search_position(int depth)
{
	//printf("depth:%d\n", depth);
	//printf("bestmove d2d4\n");

	int score = negamax(-50000, 50000, depth);


	//best_move = Mget_best_move(depth);


	printf("bestmove ");
	print_move(best_move);
	printf("\n");
}

#pragma endregion
/****************************************\
 ========================================
				 UCI
 ========================================
\****************************************/
#pragma region

//example e7e8q
int parse_move(const char* move_string)
{
	int length = strlen(move_string);
	if(length < 4)
		throw std::invalid_argument("move string too short!");

	Moves move_list[1];

	generate_moves(move_list);

	//parse source        //x                      //y
	int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8 ;
	//parse target        //x                      //y
	int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;


	//go over moves in current position
	for (int i = 0; i < move_list->count; i++)
	{
		int move = move_list->moves[i];

		//sourse and target matches engine move
		if (source_square == get_move_source(move) && target_square == get_move_target(move))
		{
			int promoted_piece = get_move_promoted(move);

			//check if promoted piece matches engine move
			if (promoted_piece)
			{
				if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
					return move;
				if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
					return move;
				if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
					return move;
				if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
					return move;

				//skip on wrong promotion
				continue;
			}
			return move;
		}
	}

	//illegal move
	return 0;
}

void parse_position(const char* command)
{
	//move pointer to next token
	command += 9;

	const char* current = command;

	//parse startpos
	//if command == startpos
	if (strncmp(command, "startpos", 8) == 0)
	{
		//init startpos
		parse_fen(start_position);
	}
	else//command == fen
	{
		//make sure fen is in string
		          
		//finds substring fen
		current = strstr(command, "fen");

		if (current == NULL)//if not found
			parse_fen(start_position);
		else//found
		{
			current += 4;
		
			parse_fen(current);
		}
		//printf("1:%s\n", command);
		//printf("2:%s\n", current);
	}

	//parse moves after initial position is set up
	current = strstr(command, "moves");//finds if moves in string

	if (current != NULL)
	{
		//move to the moves
		current += 6;

		while (*current)
		{
			//parse next move
			int move = parse_move(current);

			if (move == 0)
				break;//invalide move

			make_move(move, ALL_MOVES);

			/*printf("%s\n",current);
			std::cout << "cout:" << *current << std::endl;*/

			//move the pointer to next move
			while (*current && *current != ' ') ++current;

			current++;//skip space
		}
		//printf("Cur:%s\n", current);
	}
	//print_board(true);
}

void parse_go(const char* command)
{
	int depth = -1;

	const char* current_depth = NULL;

	//fixed depth search
	//set curr_depth to the pntr before depth
	if (current_depth = strstr(command, "depth"))
	{
		//converts string to int - skip token & space after
		depth = atoi(current_depth + 6);
	}
	else//handle time controls
	{
		depth = 4;
	}

	//printf("depth:%d\n", depth);

	search_position(depth);
}


/*
* GUI -> isready
* Engine -> readyok
* GUI -> ucinewgame
* 
*/

//main uci loop


void uci_loop()
{
	const int size = 4095;

	//reset stdin &stdin buffers
	//setvbuf(stdin, NULL, _IOFBF, 64);
	//setvbuf(stdout, NULL, _IOFBF, 64);

	//setbuf(stdin, NULL);
	//setbuf(stdout, NULL);

	//user/GUI buffer
	char input[size];

	//engine info
	printf("Crusher Chess\n");
	printf("Hanno Kruger\n");
	printf("uciok\n");

	//uci loop
	while(1)
	{
		//reset input
		memset(input, 0, sizeof(input));

		//make sure output reaches gui
		fflush(stdout);

		//skip loop cycle while nothing in stdin else populate input
		if (!fgets(input, size, stdin)) continue;	
	
		//skip newline
		if (input[0] == '\n') continue;

		//UCI isready
		if (strncmp(input, "isready", 7) == 0)
		{
			printf("readyok\n");
			continue;
		}
		//UCI position		 
		else if (strncmp(input, "position", 8) == 0)
			parse_position(input);
		//UCI new game
		else if (strncmp(input, "ucinewgame", 10) == 0)
			parse_position("position startpos");
		//UCI go
		else if (strncmp(input, "go", 2) == 0)
			parse_go(input);
		//UCI quit
		else if (strncmp(input, "quit", 4) == 0)
			break;
		//greetings
		else if (strncmp(input, "uci", 3) == 0)
		{
			//engine info
			printf("Crusher Chess\n");
			printf("Hanno Kruger\n");
			printf("uciok\n");
		}
		else if (input[0] == 'd')
			print_board();
		else
			printf("Unknown command\n");
	}
}


#pragma endregion
/****************************************\
 ========================================
				 Main
 ========================================
\****************************************/

int main()
{   //use %ls for wchar
	
	init_all();	
	
	
	int debug = 0;

	if (debug)
	{
		parse_fen(start_position);
		print_board();
		search_position(4);
		//perft_test(5);

		//print_board();
		//printf("score: %d\n", evaluate());

	}
	else
		uci_loop();
	

	std::wcin.get();
	return 0;
}










void Test1(int size)
{
	U64 num = 0xffffffffffffffff;
	auto numcopy = num;

	int res = 0;

	auto startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < size; i++)
	{
		res = 0;

		while (num)
		{
			num &= num - 1;
			++res;
		}


		num = --numcopy;
	}
	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	auto elapsedMS = ((long double)elapsedMicro) / 1000;


	printf("Method 1 Time:%.2LF (MS)\n", elapsedMS);

	printf("%d\n", res);
}
void Test2(int size)
{
	U64 num = 0xffffffffffffffff;
	int res = 0;

	auto startTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < size; i++)
	{
		res = std::popcount(num);
		num--;
	}
	auto endTime = std::chrono::high_resolution_clock::now();

	auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	auto elapsedMS = ((long double)elapsedMicro) / 1000;


	printf("Method 2 Time:%.2LF (MS)\n", elapsedMS);

	printf("%d\n", res);
}

void rec(int num)
{
	if (num < 1)
		return;
	else
	{
		std::cout << num << " ";
		rec(num - 1);
		std::cout << num << " ";
		return;
	}
}

std::stack<int> stack;

void norec(int num)
{
	while (num > 0)
	{
		std::cout << num << " ";

		stack.push(num);

		num--;
	}
	while (!stack.empty())
	{
		std::cout << stack.top() << " ";
		stack.pop();
	}
}