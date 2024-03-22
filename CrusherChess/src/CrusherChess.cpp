//#define DEBUG

//includes
#pragma region 


#define NOMINMAX

#include <stack>
#include <iostream>
#include <cassert>
#include <string>
#include <io.h>
#include <chrono>
#include <wtypes.h>
#include <vector>
#include <ostream>

#include "types.h"
#include "random.h"

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>& custom_endl(std::basic_ostream<CharT, Traits>& os) {
    return std::cout << std::endl;
}


const char* VERSION = "1.3.1";
#define MAX_HASH 32768

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
U64 mask_pawn_attacks(bool side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 bishop_blocker_attacks(int square, U64 block);
U64 rook_blocker_attacks(int square, U64 block);
void init_leapers_attacks();
void init_hash_table(int mb);
void init_all();


U64 generate_magic_number();

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
void init_sliders_attacks(int bishop);

static inline U64 get_bishop_attacks(int square, U64 occupancy);
static inline U64 get_rook_attacks(int square, U64 occupancy);
static inline bool is_square_attacked(int square, int side);

static inline int score_move(int move);
void clear_hash_table();

//void search_position(int depth);
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


static inline int get_lsb_index(U64 num)
{
	//-num = (0 - num) = (~num + 1ULL) -> flip bits & add one

	assert(num != 0ULL);
	
	return count_bits((num & (0 - num)) - 1);	
}
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

//"almost" unique position identifier aka hash key
U64 hash_key = 0ULL;

U64 repetition_table[1000] = {};//number of plies in whole game
int repetition_index = 0;
//half move counter
int ply = 0;


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

//masks for pawns [square]
U64 file_masks[SQUARE_NB];
U64 rank_masks[SQUARE_NB];

U64 isolated_masks[SQUARE_NB];
U64 passed_masks[COLOR_NB][SQUARE_NB];

#pragma endregion
/****************************************\
 ========================================
			 Zobrist keys
 ========================================
\****************************************/
#pragma region

//Random piece keys [piece][square]
U64 piece_keys[12][SQUARE_NB];

//Random enpassant keys
U64 enpassant_keys[SQUARE_NB];

//Random castling keys
U64 castle_keys[16];

//random side key
U64 side_key;

void init_random_keys()
{
    //1804289383;
    PRNG rng(1070372);

	//loop over piece codes
	for (int piece = P; piece <= k; piece++)
	{
		//assign random piece key
		for (int square = 0; square < SQUARE_NB; square++)
			piece_keys[piece][square] = rng.rand<U64>();
	}
	//loop over board
	for (int square = 0; square < SQUARE_NB; square++)
		enpassant_keys[square] = rng.rand<U64>();

	//init castle keys
	for (int i = 0; i < 16; i++)
		castle_keys[i] = rng.rand<U64>();

	//init random side key	
	side_key = rng.rand<U64>();
}

#pragma endregion
/****************************************\
 ========================================
				Hasing
 ========================================
\****************************************/
#pragma region

//generate almost unique ID
U64 generate_hash_key()
{
	U64 final_key = 0ULL;
	U64 bitboard;

	for (int piece = P; piece <= k; piece++)
	{
		bitboard = bitboards[piece];

		//loop over pieces
		while (bitboard)
		{
			int square = get_lsb_index(bitboard);

			//hash piece
			final_key ^= piece_keys[piece][square];

			//pop lsb
			pop_bit(bitboard, square);
		}
	}
	//hash enpasant
	if (enpassant != NO_SQ)//Can remove this?
		final_key ^= enpassant_keys[enpassant];
	//hash castle	
	final_key ^= castle_keys[castle];

	if (side == BLACK)
		final_key ^= side_key;

	return final_key;
}

#pragma endregion
/****************************************\
 ========================================
		   Debug + Input & output
 ========================================
\****************************************/
#pragma region

int get_piece(int file, int rank)
{
	int piece = -1;

	for (int i = P; i <= k; i++)
	{
		if (get_bit(bitboards[i], (rank * 8 + file)))
		{
			piece = i;
			break;
		}
	}
	return piece;
}

std::string get_fen()
{
	std::ostringstream ss;

	for (int r = 0; r < 8; r++)
	{
		for (int f = 0; f < 8; f++)
		{
			int empty = 0;

			while (get_piece(f, r) == -1 && f++ < 8)
				empty++;

			if (empty)
				ss << empty;

			if (f <= 7)
			{
				int piece = get_piece(f, r);

				if (piece == -1)
					assert(!"piece is -1");

				ss << ascii_pieces[piece];
			}
		}

		if (r < 7)
			ss << '/';
	}

	ss << (side == WHITE ? " w " : " b ");

	if (castle & 1)		ss << 'K';
	if (castle & 2)		ss << 'Q';
	if (castle & 4)		ss << 'k';
	if (castle & 8)		ss << 'q';
	if (!(castle & 15))	ss << '-';

	ss << (enpassant == NO_SQ ? " - " : " " + std::string(square_to_coordinates[enpassant]) + " ");

	//need to add half and full move counters
	ss << "0 " << "1";

	return ss.str();
}

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

//					if (piece == P)
//						WriteConsoleW(handle, unicode_pieces[piece], 1, &written, NULL);
//					else
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

	printf("Side: %s\n", side ? "Black" : "White");

	printf("En Passant: %s\n", (enpassant != NO_SQ) ? (square_to_coordinates[enpassant]) : "no");

	printf("Castling: %c%c%c%c\n",
		(castle & WK) ? 'K' : '-',
		(castle & WQ) ? 'Q' : '-',
		(castle & BK) ? 'k' : '-',
		(castle & BQ) ? 'q' : '-'
	);

	std::cout << "Fen: " << get_fen() << custom_endl;

	printf("Key: %llx\n\n", hash_key);
}

void parse_fen(const char* fen)
{
	// reset board pos and state
	memset(bitboards, 0ULL, sizeof(bitboards));
	memset(occupancies, 0ULL, sizeof(occupancies));

	castle = 0;
	repetition_index = 0;

	//ply and should be zero here
	assert(ply == 0);

	//maby not nessecary?
	memset(repetition_table, 0, sizeof(repetition_table));


	for (int rank = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			int square = rank * 8 + file, piece = -1;

			//parse chars
			if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
			{
                piece = char_pieces.at(*fen);
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
            default: std::cout << ("fen string in inccorect format!") << custom_endl; break;
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

	//init hash key
	hash_key = generate_hash_key();

}

//for uci
std::string get_move_string(int move)
{
	if (get_move_promoted(move))
		return std::format("{}{}{}",
			square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)],
			promoted_pieces.at(get_move_promoted(move)));
	else
		return std::format("{}{}",
			square_to_coordinates[get_move_source(move)],
			square_to_coordinates[get_move_target(move)]);
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

//set file or rank
U64 set_file_rank_mask(int file_no, int rank_no)
{
	U64 mask = 0ULL;

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++, square++)
		{

			if (file == file_no || rank == rank_no)
			{
				//printf("file: %d   rank:%d\n", file, rank);
				mask |= set_bit(mask, square);
			}
		}
	}

	return mask;
}

//init evaluation masks
void init_evaluation_masks()
{
	//use -1 to not set a file
	U64 mask = set_file_rank_mask(-1, -1);

	for (int rank = 0, square = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++, square++)
		{
			file_masks[square] |= set_file_rank_mask(file, -1);
			rank_masks[square] |= set_file_rank_mask(-1, rank);

			isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
			isolated_masks[square] |= set_file_rank_mask(file + 1, -1);



			passed_masks[WHITE][square] |= set_file_rank_mask(file - 1, -1);
			passed_masks[WHITE][square] |= set_file_rank_mask(file, -1);
			passed_masks[WHITE][square] |= set_file_rank_mask(file + 1, -1);

			passed_masks[BLACK][square] |= passed_masks[WHITE][square];
			passed_masks[BLACK][square] |= passed_masks[WHITE][square];
			passed_masks[BLACK][square] |= passed_masks[WHITE][square];

			//loop over redundant ranks and reset redudant bits 
			for (int i = 0; i < (8 - rank); i++)
			{
				passed_masks[WHITE][square] &= ~rank_masks[(7 - i) * 8 + file];
				passed_masks[BLACK][square] &= ~rank_masks[i * 8 + file];
			}

			//print_bitboard(passed_masks[WHITE][square]);
		}
	}
}


void init_all()
{
	init_leapers_attacks();
	init_sliders_attacks(BISHOP);
	init_sliders_attacks(ROOK);
	init_random_keys();
	init_evaluation_masks();
	//init hash table with default 64mb
	init_hash_table(64);

	parse_fen(start_position);
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
    PRNG rng(1070372);

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
		U64 magic_number = rng.sparse_rand<U64>();

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

void init_sliders_attacks(int piece)
{
	for (int square = 0; square < 64; square++)
	{
		//init bishop & rook masks
		bishop_masks[square] = mask_bishop_attacks(square);
		rook_masks[square] = mask_rook_attacks(square);

		//init current mask
		U64 attack_mask = (piece == BISHOP) ? bishop_masks[square] : rook_masks[square];

		//init occupancy bit count
		int relevant_bits_count = count_bits(attack_mask);

		//init occupancy indicies
		int occupancy_indicies = (1 << relevant_bits_count);

		//loop over occupancy
		for (int i = 0; i < occupancy_indicies; i++)
		{
			if (piece == BISHOP)
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

							add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
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
				attacks = get_bishop_attacks(source_square, occupancies[BOTH]) & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

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

static inline int make_move(int move)
{
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

	//hash piece (remove and set) 
	hash_key ^= piece_keys[m_piece][m_source_square];
	hash_key ^= piece_keys[m_piece][m_target_square];


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
			//if piece on target square 
			if (get_bit(bitboards[bb_piece], m_target_square))
			{
				//remove the piece
				pop_bit(bitboards[bb_piece], m_target_square);

				//remove piece from hash key
				hash_key ^= piece_keys[bb_piece][m_target_square];

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
			// remove pawn from hash key
			hash_key ^= piece_keys[P][m_target_square];
		}
		else
		{
			pop_bit(bitboards[p], m_target_square);
			// remove pawn from hash key
			hash_key ^= piece_keys[p][m_target_square];
		}

		// set promoted piece on chess board and hash it
		set_bit(bitboards[m_promoted_piece], m_target_square);
		hash_key ^= piece_keys[m_promoted_piece][m_target_square];
	}

	if (m_enpassant)
	{
		// white to move
		if (side == WHITE)
		{
			// remove captured pawn
			pop_bit(bitboards[p], m_target_square + 8);

			// remove pawn from hash key
			hash_key ^= piece_keys[p][m_target_square + 8];
		}
		else
		{
			// remove captured pawn
			pop_bit(bitboards[P], m_target_square - 8);

			// remove pawn from hash key
			hash_key ^= piece_keys[P][m_target_square - 8];
		}
	}

	//hash enpassant
	if (enpassant != NO_SQ)	hash_key ^= enpassant_keys[enpassant];

	//always reset enpassant
	enpassant = NO_SQ;


	//for enpassant
	if (m_double_push)
	{
		// white to move
		if (side == WHITE)
		{
			enpassant = m_target_square + 8;

			//hash enpassant
			hash_key ^= enpassant_keys[m_target_square + 8];
		}

		// black to move
		else
		{
			enpassant = m_target_square - 8;

			//hash enpassant
			hash_key ^= enpassant_keys[m_target_square - 8];
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
			//hash rook (remove, set)
			hash_key ^= piece_keys[R][h1];
			hash_key ^= piece_keys[R][f1];
			break;

			//WQ
		case c1:
			pop_bit(bitboards[R], a1);//remove rook
			set_bit(bitboards[R], d1);//set rook	
			//hash rook (remove, set)
			hash_key ^= piece_keys[R][a1];
			hash_key ^= piece_keys[R][d1];
			break;

			//BK
		case g8:
			pop_bit(bitboards[r], h8);//remove rook
			set_bit(bitboards[r], f8);//set rook
			//hash rook (remove, set)
			hash_key ^= piece_keys[r][h8];
			hash_key ^= piece_keys[r][f8];
			break;

			//BQ
		case c8:
			pop_bit(bitboards[r], a8);//remove rook
			set_bit(bitboards[r], d8);//set rook
			//hash rook (remove, set)
			hash_key ^= piece_keys[r][a8];
			hash_key ^= piece_keys[r][d8];
			break;
		}
	}

	//hash castling go previous
	hash_key ^= castle_keys[castle];

	//update castling rights
	castle &= castling_rights[m_source_square];
	castle &= castling_rights[m_target_square];

	//hash castling go current
	hash_key ^= castle_keys[castle];

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
	//hash side
	hash_key ^= side_key;


	//=====debug hash key incremental update======
	/*U64 hash_from_scratch = generate_hash_key();

	if (hash_key != hash_from_scratch)
	{
		printf("Make move\n");
		printf("move: ");
		std::cout << get_move_string(move) << custom_endl;
		print_board();

		printf("hash sould be: %llx\n", hash_from_scratch);

		getchar();
	}*/

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


#pragma endregion
/****************************************\
 ========================================
				 Perft
 ========================================
\****************************************/
#pragma region

//std::set<U64> keys;
//std::vector<U64> collisions;


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
		if (!make_move(move_list->moves[move_count]))
		{
			take_back();
			continue;
		}

		//auto t = keys.insert(hash_key);

		//if (t.second)
		//{
			//does not exist in set
		//}
		//else
		//{
			//collisions.push_back(hash_key);
			//std::cout << "Collision! " << "length:" << keys.size() << custom_endl;

			//print_board();
		//}

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
			if (!make_move(move))
			{
				take_back();
				continue;
			}

			//auto t = keys.insert(hash_key);

			//if (t.second)
			//{
				//does not exist in set
			//}
			//else
			//{
				//std::cout << "Collision! " << "length:" << keys.size() << custom_endl;
			//	collisions.push_back(hash_key);

				//print_board();
			//}

			U64 cumulative_nodes = nodes;

			// call perft driver recursively
			perft_driver(depth - 1);

			U64 current_nodes = nodes - cumulative_nodes;

			// take back
			take_back();

            const int promoted = get_move_promoted(move);
            std::cout <<
            square_to_coordinates[get_move_source(move)] <<
            square_to_coordinates[get_move_target(move)] <<
            (promoted ? promoted_pieces.at(promoted) : ' ') <<
            " nodes: " << current_nodes << custom_endl;
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
			  Time control
 ========================================
\****************************************/
#pragma region

bool quit = false;//exit engine flag
bool time_set = false;//time control availability
bool stopped = false;//when time is up
U64 start_time = 0ULL;//uci starttime command
U64 stop_time = 0ULL;//uci stoptime command


/*
  Function to "listen" to GUI's input during search.
  It's waiting for the user input from STDIN.
  OS dependent.

  First Richard Allbert aka BluefeverSoftware grabbed it from somewhere...
  And then Code Monkey King has grabbed it from VICE)

*/

int input_waiting()
{
	//must be windows

	//linux
	return 0;

	static int init = 0, pipe;
	static HANDLE inh;
	DWORD dw;

	if (!init)
	{
		init = 1;
		inh = GetStdHandle(STD_INPUT_HANDLE);
		pipe = !GetConsoleMode(inh, &dw);
		if (!pipe)
		{
			SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(inh);
		}
	}

	if (pipe)
	{
		if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
		return dw;
	}
	else
	{
		GetNumberOfConsoleInputEvents(inh, &dw);
		return dw <= 1 ? 0 : dw;
	}
}

// read GUI/user input
void read_input()
{
	// bytes to read holder
	int bytes;

	// GUI/user input
	char input[256] = "", * endc;

	// "listen" to STDIN
	if (input_waiting())
	{
		// tell engine to stop calculating
		stopped = true;

		// loop to read bytes from STDIN
		do
		{
			// read bytes from STDIN
			bytes = _read(_fileno(stdin), input, 256);
		}
		while (bytes < 0);//read until bytes available then break

		// searches for the first occurrence of '\n'
		endc = strchr(input, '\n');

		// if found new line set value at pointer to 0
		if (endc) *endc = 0;

		// if input is available
		if (strlen(input) > 0)
		{
			// match UCI "quit" command //strncmp returns 0 if string are equal
			if (!strncmp(input, "quit", 4) || !strncmp(input, "stop", 4))
			{
				// tell engine to terminate exacution    
				quit = true;
			}

			
			//else if (!strncmp(input, "stop", 4))
			//{
			//	// tell engine to terminate exacution
			//	quit = true;
			//}
		}
	}
}

// a bridge function to interact between search and GUI input
static void communicate()
{
	// if time is up break here
	if (time_set == 1 && GetTickCount64() > stop_time)
	{
		// tell engine to stop calculating
		stopped = true;
	}

	// read GUI input
	read_input();
}

#pragma endregion
/****************************************\
 ========================================
			   Simple Search
 ========================================
\****************************************/
#pragma region

const int piece_scores[12] = { 10, 30, 31, 50, 90, 2000, -10, -30, -31, -50, -90, -2000 };


static inline int Mposition_evaluate(int cnt, int depth)
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
		if (!make_move(moves->moves[i]))
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
		return Mposition_evaluate(cnt, depth);

	if (Maximizing)//white
	{
		int maxeval = INT_MIN;

		for (int i = 0; i < moves->count; i++)
		{
			copy_board();

			if (!make_move(moves->moves[i]))
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

			if (!make_move(moves->moves[i]))
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

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, false);

            std::cout << "no" << (i + 1) << "  score: " << eval << "  move:";
			std::cout << get_move_string(moves->moves[i]) << custom_endl;

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

			if (!make_move(moves->moves[i]))
			{
				take_back();
				continue;
			}

			int eval = MMiniMax(depth - 1, true);
			printf("no%d  score: %d  move:", (i + 1), eval);
			std::cout << get_move_string(moves->moves[i]) << custom_endl;

			take_back();

			if (eval <= bestScore)
			{
				if (eval == bestScore)
					bestmoves.push_back(moves->moves[i]);
				else
				{
                    std::cout << "black new eval:" << eval << custom_endl;

					bestmoves.clear();
					bestmoves.push_back(moves->moves[i]);
					bestScore = eval;
				}
			}
		}
	}
	return bestmoves[rand() % bestmoves.size()];
}

void Msearch_position(int depth)
{
	printf("depth:%d\n", depth);

	int best = Mget_best_move(depth);

    std::cout << "bestmove " << get_move_string(best);
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



// material score [game phase][piece]
const int material_score[2][12] =
{
	// opening material score
	82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000,

	// endgame material score
	94, 281, 297, 512,  936, 12000, -94, -281, -297, -512,  -936, -12000
};

// game phase scores
const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

// game phases
enum { OPENING, ENDGAME, MIDDELGAME };



// positional piece scores [game phase][piece][square]
const int positional_score[2][6][SQUARE_NB] =

// opening positional piece scores //
{
	//pawn
	0,   0,   0,   0,   0,   0,  0,   0,
	98, 134,  61,  95,  68, 126, 34, -11,
	-6,   7,  26,  31,  65,  56, 25, -20,
	-14,  13,   6,  21,  23,  12, 17, -23,
	-27,  -2,  -5,  12,  17,   6, 10, -25,
	-26,  -4,  -4, -10,   3,   3, 33, -12,
	-35,  -1, -20, -23, -15,  24, 38, -22,
	0,   0,   0,   0,   0,   0,  0,   0,

	// knight
	-167, -89, -34, -49,  61, -97, -15, -107,
	-73, -41,  72,  36,  23,  62,   7,  -17,
	-47,  60,  37,  65,  84, 129,  73,   44,
	-9,  17,  19,  53,  37,  69,  18,   22,
	-13,   4,  16,  13,  28,  19,  21,   -8,
	-23,  -9,  12,  10,  19,  17,  25,  -16,
	-29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,

	// bishop
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	-4,   5,  19,  50,  37,  37,   7,  -2,
	-6,  13,  13,  26,  34,  12,  10,   4,
	0,  15,  15,  15,  14,  27,  18,  10,
	4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,

	// rook
	32,  42,  32,  51, 63,  9,  31,  43,
	27,  32,  58,  62, 80, 67,  26,  44,
	-5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,

	// queen
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	-9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	-1, -18,  -9,  10, -15, -25, -31, -50,

	// king
	-65,  23,  16, -15, -56, -34,   2,  13,
	29,  -1, -20,  -7,  -8,  -4, -38, -29,
	-9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,


	// Endgame positional piece scores //

	//pawn
	0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	94, 100,  85,  67,  56,  53,  82,  84,
	32,  24,  13,   5,  -2,   4,  17,  17,
	13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	4,   7,  -6,   1,   0,  -5,  -1,  -8,
	13,   8,   8,  10,  13,   0,   2,  -7,
	0,   0,   0,   0,   0,   0,   0,   0,

	// knight
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,

	// bishop
	-14, -21, -11,  -8, -7,  -9, -17, -24,
	-8,  -4,   7, -12, -3, -13,  -4, -14,
	2,  -8,   0,  -1, -2,   6,   0,   4,
	-3,   9,  12,   9, 14,  10,   3,   2,
	-6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,

	// rook
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	7,  7,  7,  5,  4,  -3,  -5,  -3,
	4,  3, 13,  1,  2,   1,  -1,   2,
	3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,

	// queen
	-9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,

	// king
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	10,  17,  23,  15,  20,  45,  44,  13,
	-8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};

// mirror positional score tables for opposite side only flip vertical
const int mirror_square[SQUARE_NB] =
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

//// mirror positional score tables for opposite side (flips vertical and horizontal)
//const int mirror_square[SQUARE_NB] =
//{
//	h1, g1, f1, e1, d1, c1, b1, a1, 
//	h2, g2, f2, e2, d2, c2, b2, a2, 
//	h3, g3, f3, e3, d3, c3, b3, a3, 
//	h4, g4, f4, e4, d4, c4, b4, a4, 
//	h5, g5, f5, e5, d5, c5, b5, a5, 
//	h6, g6, f6, e6, d6, c6, b6, a6, 
//	h7, g7, f7, e7, d7, c7, b7, a7, 
//	h8, g8, f8, e8, d8, c8, b8, a8
//};


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



//// file masks [square]
//U64 file_masks[SQUARE_NB];
//
//// rank masks [square]
//U64 rank_masks[SQUARE_NB];
//
//// isolated pawn masks [square]
//U64 isolated_masks[SQUARE_NB];
//
//// white passed pawn masks [square]
//U64 white_passed_masks[SQUARE_NB];
//
//// black passed pawn masks [square]
//U64 black_passed_masks[SQUARE_NB];




// extract rank from a square [square]
const int get_rank[SQUARE_NB] =
{
	7, 7, 7, 7, 7, 7, 7, 7,
	6, 6, 6, 6, 6, 6, 6, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

// double pawns penalty
const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;

// isolated pawn penalty
const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

// bonusses
const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 };

// semi open file score
const int semi_open_file_score = 10;

// open file score
const int open_file_score = 15;

// mobility units (values from engine Fruit reloaded)
static const int bishop_unit = 4;
static const int queen_unit = 9;

// mobility bonuses (values from engine Fruit reloaded)
static const int bishop_mobility_opening = 5;
static const int bishop_mobility_endgame = 5;
static const int queen_mobility_opening = 1;
static const int queen_mobility_endgame = 2;

// king's shield bonus
const int king_shield_bonus = 5;


static inline int get_game_phase_score()
{
	/*
		The game phase score of the game is derived from the pieces
		(not counting pawns and kings) that are still on the board.
		The full material starting position game phase score is:

		4 * knight material score in the opening +
		4 * bishop material score in the opening +
		4 * rook material score in the opening +
		2 * queen material score in the opening
	*/

	// white & black game phase scores
	int white_piece_scores = 0, black_piece_scores = 0;

	// loop over white pieces
	for (int piece = N; piece <= Q; piece++)
		white_piece_scores += count_bits(bitboards[piece]) * material_score[OPENING][piece];

	// loop over black pieces
	for (int piece = n; piece <= q; piece++)
		black_piece_scores += count_bits(bitboards[piece]) * -material_score[OPENING][piece];

	// return game phase score
	return white_piece_scores + black_piece_scores;
}

static inline int lerp_positional_score(int piece_type,int square,int game_phase_score)
{
	return
		(
		positional_score[OPENING][piece_type][square] * game_phase_score +
		positional_score[ENDGAME][piece_type][square] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}
static inline int lerp_material_score(int piece, int game_phase_score)
{
	return
		(material_score[OPENING][piece] * game_phase_score
		+ material_score[ENDGAME][piece] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}



static inline int evaluate()
{
	int game_phase_score = get_game_phase_score();
	//printf("game phase score: %d\n", game_phase_score);
	 

	//static evaluation
	int score = 0,opening_score = 0,endgame_score = 0;

	//current pieces bitboard copy
	U64 bitboard;

	//init piece and square
	int piece, square;

	//penalties
	int double_pawns = 0;


	for (int bb_piece = P; bb_piece <= k; bb_piece++)
	{
		bitboard = bitboards[bb_piece];

		while (bitboard)
		{
			piece = bb_piece;
			square = get_lsb_index(bitboard);

			// get opening/endgame material score
			opening_score += material_score[OPENING][piece];
			endgame_score += material_score[ENDGAME][piece];

			if (piece < 6)//white
			{
				opening_score += positional_score[OPENING][piece][square];
				endgame_score += positional_score[ENDGAME][piece][square];
			}
			else		 //black    //using -6 might be faster here
			{
				opening_score -= positional_score[OPENING][piece_to_type[piece]][mirror_square[square]];
				endgame_score -= positional_score[ENDGAME][piece_to_type[piece]][mirror_square[square]];
			}

			
		    /*old method:
			//calculate the interpolated material score between opening and endgame
			if (game_phase == MIDDELGAME)		
				score += lerp_material_score(piece, game_phase_score);		
			//material score
			else
				score += material_score[game_phase][piece];	


			//calculate the interpolated positional score between opening and endgame
			if (game_phase == MIDDELGAME)
			{
				if (piece < 6)//white
					score += lerp_positional_score(piece, square, game_phase_score);
				else		  //black	 //-6 might be faster here
					score -= lerp_positional_score(piece_to_type[piece], mirror_square[square], game_phase_score);
			}
			// score material weights with pure scores in OPENING or ENDGAME
			else
			{
				if (piece < 6)//white
					score += positional_score[game_phase][piece][square];
				else		  //black    //-6 might be faster here
					score -= positional_score[game_phase][piece_to_type[piece]][mirror_square[square]];
			}
			*/


			
				
			switch (piece)
			{
				//evaluate white pawns
				case P:
				
					// double pawn penalty
					double_pawns = count_bits(bitboards[P] & file_masks[square]);

					// on double pawns (tripple, etc)
					if (double_pawns > 1)
					{
						opening_score += (double_pawns - 1) * double_pawn_penalty_opening;
						endgame_score += (double_pawns - 1) * double_pawn_penalty_endgame;
					}

					// on isolated pawn
					if ((bitboards[P] & isolated_masks[square]) == 0)
					{
						// give an isolated pawn penalty
						opening_score += isolated_pawn_penalty_opening;
						endgame_score += isolated_pawn_penalty_endgame;
					}
					// on passed pawn
					if ((passed_masks[WHITE][square] & bitboards[p]) == 0)
					{
						// give passed pawn bonus
						opening_score += passed_pawn_bonus[get_rank[square]];
						endgame_score += passed_pawn_bonus[get_rank[square]];
					}
					break;
				//evaluate white knights
				case N:
				
					break;
				//evaluate white bishops
				case B:
					// mobility
					opening_score += (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_opening;
					endgame_score += (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_endgame;
					break;
				//evaluate white rooks
				case R:
					// semi open file
					if ((bitboards[P] & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score += semi_open_file_score;
						endgame_score += semi_open_file_score;
					}

					// semi open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score += open_file_score;
						endgame_score += open_file_score;
					}
					break;
				//evaluate white queens
				case Q:
					// mobility
					opening_score += (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_opening;
					endgame_score += (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_endgame;
					break;
				//evaluate white king
				case K:
					
					// open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add open file penalty
						opening_score -= open_file_score;
						endgame_score -= open_file_score;
					}
					// semi open file
					else if ((bitboards[P] & file_masks[square]) == 0)
					{
						// add semi open file penalty
						opening_score -= semi_open_file_score;
						endgame_score -= semi_open_file_score;
					}

					// king safety bonus
					opening_score += count_bits(king_attacks[square] & occupancies[WHITE]) * king_shield_bonus;
					endgame_score += count_bits(king_attacks[square] & occupancies[WHITE]) * king_shield_bonus;

					break;
				//evaluate black pawns
				case p:
					// double pawn penalty
					double_pawns = count_bits(bitboards[p] & file_masks[square]);

					// on double pawns (tripple, etc)
					if (double_pawns > 1)
					{
						opening_score -= (double_pawns - 1) * double_pawn_penalty_opening;
						endgame_score -= (double_pawns - 1) * double_pawn_penalty_endgame;
					}

					// on isolated pawn
					if ((bitboards[p] & isolated_masks[square]) == 0)
					{
						// give an isolated pawn penalty
						opening_score -= isolated_pawn_penalty_opening;
						endgame_score -= isolated_pawn_penalty_endgame;
					}
					// on passed pawn
					if ((passed_masks[BLACK][square] & bitboards[P]) == 0)
					{
						// give passed pawn bonus
						opening_score -= passed_pawn_bonus[get_rank[square]];
						endgame_score -= passed_pawn_bonus[get_rank[square]];
					}
					break;
				//evaluate black knights
				case n:
				
					break;
					//evaluate black bishops
				case b:
					// mobility
					opening_score -= (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_opening;
					endgame_score -= (count_bits(get_bishop_attacks(square, occupancies[BOTH])) - bishop_unit) * bishop_mobility_endgame;
					break;
				//evaluate black rooks
				case r:
					// semi open file
					if ((bitboards[p] & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score -= semi_open_file_score;
						endgame_score -= semi_open_file_score;
					}

					// semi open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						// add semi open file bonus
						opening_score -= open_file_score;
						endgame_score -= open_file_score;
					}
					break;
				//evaluate black queens
				case q:
					// mobility
					opening_score -= (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_opening;
					endgame_score -= (count_bits(get_queen_attacks(square, occupancies[BOTH])) - queen_unit) * queen_mobility_endgame;
					break;
				//evaluate black king
				case k:
					//open file
					if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
					{
						//open file penalty
						opening_score += open_file_score;
						endgame_score += open_file_score;
					}
					// semi open file
					else if ((bitboards[p] & file_masks[square]) == 0)
					{
						//add semi open file penalty
						opening_score += semi_open_file_score;
						endgame_score += semi_open_file_score;
					}		

					//king safety bonus
					opening_score -= count_bits(king_attacks[square] & occupancies[BLACK]) * king_shield_bonus;
					endgame_score -= count_bits(king_attacks[square] & occupancies[BLACK]) * king_shield_bonus;
					break;
			}
			

			//pop lsb bit
			pop_bit(bitboard, square);
		}
	}


	/*
				Now in order to calculate interpolated score
				for a given game phase we use this formula
				(same for material and positional scores):

				(
				  score_opening * game_phase_score +
				  score_endgame * (opening_phase_score - game_phase_score)
				) / opening_phase_score

				E.g. the score for pawn on d4 at phase say 5000 would be
				interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
			*/



			//int game_phase;

			////game > opening
			//if (game_phase_score > opening_phase_score) game_phase = OPENING;
			////game < endgame
			//else if (game_phase_score < endgame_phase_score) game_phase = ENDGAME;
			////game between opening and endgame 
			//else game_phase = MIDDELGAME;


	// return pure opening score in opening
	if (game_phase_score > opening_phase_score) score = opening_score;
	// return pure endgame score in engame
	else if (game_phase_score < endgame_phase_score) score = endgame_score;
	// interpolate score in the middlegame
	else
	 score = (
		 opening_score * game_phase_score +
		 endgame_score * (opening_phase_score - game_phase_score)
		 ) / opening_phase_score;


	return (side == WHITE) ? score : -score;
}




#pragma endregion
/****************************************\
 ========================================
		 Transposition Table
 ========================================
\****************************************/
#pragma region

//score layout
//[-inf...-mate_value...-mate_score...normal(non mate)score...mate_score...mate_value...inf]

#define infinity 50000
#define mate_value 49000
#define mate_score 48000

// hash table size (16mb)
//#define tt_size 16
U64 hash_entries = 0ULL;

#define no_hash_entry infinity * 2

// transposition table hash flags
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2


// transposition table data structure
typedef struct TT
{
	U64 key;	//8 byte	"almost" unique chess position identifier
	int depth;  //4 byte    current search depth
	int flag;   //4 byte    flag the type of node (fail-low/fail-high/PV) 
	int score;  //4 byte    score (alpha/beta/PV)
} TT;      //sum:20 byte   (TT aka hash table)

// define TT instance
TT* hash_table = nullptr;

// dynamically allocate memory for hash table
void init_hash_table(int mb)
{
	const U64 ONE_MB = 0x100000ULL;

	// init hash size
	U64 hash_size = ONE_MB * mb;

	// init number of hash entries
	hash_entries = hash_size / sizeof(TT);

	// free hash table if not empty
	if (hash_table != nullptr)
    {
        std::cout << "Clearing hash memory..." << custom_endl;

		free(hash_table);
	}

	// allocate memory
	hash_table = (TT*)malloc(hash_entries * sizeof(TT));

	// if allocation has failed
	if (hash_table == nullptr)
	{
        std::cout << "Couldn't allocate memory for hash table, trying " << mb / 2 << "MB... " << custom_endl;

		// try to allocate with half size
		init_hash_table(mb / 2);
	}
	// if allocation succeeded
	else
	{
		// clear hash table
		clear_hash_table();
        std::cout << "Hash table is initialized with " << hash_entries << " entries" << custom_endl;
	}
}

// clear TT (hash table)
void clear_hash_table()
{
	// init hash table entry pointer
	TT* hash_entry;

	// loop over TT elements
	for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry++)
	{
		// reset TT inner fields
		hash_entry->key = 0;
		hash_entry->depth = 0;
		hash_entry->flag = 0;
		hash_entry->score = 0;
	}

	//this propably wont work for dynamic array
	//memset(hash_table, 0, sizeof(hash_table));

	assert(hash_table[0].depth == 0);
	assert(hash_table[0].flag == 0);
	assert(hash_table[0].key == 0);
	assert(hash_table[0].score == 0);
}

static inline int read_hash_entry(int alpha, int beta, int depth)
{
	//bring hash key down to table index storing the score data for current board position if available
	TT* hash_entry = &hash_table[hash_key % hash_entries];

	//make sure we're dealing with correct position
	if (hash_entry->key == hash_key)
	{
		//make sure to match depth our search is at
		if (hash_entry->depth >= depth)
		{
			int score = hash_entry->score;

			//retrieve score independant from actual path
			//from root position to current position
			if (score <= -mate_score) score += ply;
			if (score >= mate_score) score -= ply;

			//match the exact score (PV node)
			if (hash_entry->flag == hash_flag_exact)
				return score;
			//match alpha score (fail low node)
			if (hash_entry->flag == hash_flag_alpha && score <= alpha)
				return alpha;
			//match beta score (fail high node)
			if (hash_entry->flag == hash_flag_beta && score >= beta)
				return beta;

		}
	}
	//entry doesnt exist
	return no_hash_entry;
}

static inline void write_hash_entry(int score, int depth, int hash_flag)
{
	//bring hash key down to table index storing the score data for current board position if available
	TT* hash_entry = &hash_table[hash_key % hash_entries];

	//adjust score to find mate in shortest path
	//store score independant from actual path
	//from root position to current position
	if (score <= -mate_score) score -= ply;
	if (score >= mate_score) score += ply;

	//write hash entry data
	hash_entry->key = hash_key;
	hash_entry->score = score;
	hash_entry->flag = hash_flag;
	hash_entry->depth = depth;
}

#pragma endregion
/****************************************\
 ========================================
				Search
 ========================================
\****************************************/
#pragma region

/*
   most valuable victim & less valuable attacker

	(Victims) Pawn Knight Bishop   Rook  Queen   King
  (Attackers)
		Pawn   105    205    305    405    505    605
	  Knight   104    204    304    404    504    604
	  Bishop   103    203    303    403    503    603
		Rook   102    202    302    402    502    602
	   Queen   101    201    301    401    501    601
		King   100    200    300    400    500    600
*/

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};


//max reachable ply
#define MAX_PLY 64
//interval for when to communicate with gui
#define LISTEN 16383 //32767 //2047


//killer moves [id][ply]
int killer_moves[2][MAX_PLY] = {};

//history moves [piece][square]
int history_moves[12][SQUARE_NB] = {};


/*
	  ================================
			Triangular PV table
	  --------------------------------
		PV line: e2e4 e7e5 g1f3 b8c6
	  ================================
		   0    1    2    3    4    5

	  0    m1   m2   m3   m4   m5   m6

	  1    0    m2   m3   m4   m5   m6

	  2    0    0    m3   m4   m5   m6

	  3    0    0    0    m4   m5   m6

	  4    0    0    0    0    m5   m6

	  5    0    0    0    0    0    m6
*/

// PV length [ply[
int pv_length[MAX_PLY] = {};

// PV table [ply][ply]
int pv_table[MAX_PLY][MAX_PLY] = {};

int follow_pv = 0, score_pv = 0;

int move_scores[255] = {};


static inline void enable_pv_scoring(Moves* move_list)
{
	//disable
	follow_pv = FALSE;

	//loop over moves
	for (int i = 0; i < move_list->count; i++)
	{
		//make sure we hit pv
		if (pv_table[0][ply] == move_list->moves[i])
		{
			//enable move scoring
			score_pv = TRUE;
			//enable pv
			follow_pv = TRUE;
		}
	}
}

/*
* ===================
*    Move ordering
* ===================
*
* 1 PV move
* 2 Captures and MVA/LVA
* 3 1st killer move
* 4 2nd killer move
* 5 History moves
* 6 Unsorted moves
*/

static inline int score_move(int move)
{
	//if PV scoring allowed
	if (score_pv)
	{
		if (pv_table[0][ply] == move)
		{
			//disable flag
			score_pv = FALSE;

			//printf("PV move: ");
            //std::cout << get_move_string(move);
			//printf(" ply: %d\n", ply);

			//score PV move to search first
			return 20000;
		}
	}


	if (get_move_capture(move))
	{
		int target_piece = P;

		int start_piece, end_piece;

		int target_square = get_move_target(move);

		//white to move
		if (side == WHITE) { start_piece = p; end_piece = k; }
		//black to move
		else { start_piece = P;	end_piece = K; }

		//loop only over oppsite side pieces
		for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
		{
			//if piece on target square remove it
			if (get_bit(bitboards[bb_piece], target_square))
			{
				target_piece = bb_piece;
				break;
			}
		}
		//score capture
		return mvv_lva[get_move_piece(move)][target_piece] + 10000;
	}
	else//score quiet move
	{
		//score ist killer move
		if (killer_moves[0][ply] == move)
			return 9000;
		//score 2nd killer  move
		else if (killer_moves[1][ply] == move)
			return 8000;
		//score history move
		else
			return history_moves[get_move_piece(move)][get_move_target(move)];
	}

	return 0;
}

static inline void sort_moves(Moves* move_list)
{
	for (int i = 0; i < move_list->count; i++)
		move_scores[i] = score_move(move_list->moves[i]);

	//bubble sort
	for (int i = 0; i < move_list->count; i++)
	{
		for (int j = i + 1; j < move_list->count; j++)
		{
			if (move_scores[i] < move_scores[j])
			{
				//swap scores
				std::swap(move_scores[i], move_scores[j]);
				//swap moves
				std::swap(move_list->moves[i], move_list->moves[j]);
			}
		}
	}
}

//repetition detection
static inline bool is_repetition()
{
	//loop over repetition indices
	for (int i = 0; i < repetition_index; i++)
	{
		//if same hash key 
		if (repetition_table[i] == hash_key)
			return true;
	}

	//if no repetition found
	return false;
}


//alpha = maximizing
//beta = minimizing
static inline int quiescence(int alpha, int beta)
{
	//every time to listen to gui / user
	if ((nodes & LISTEN) == 0)
		communicate();

	nodes++;


	//make sure no overflow on arrays
	if (ply > MAX_PLY - 1)
	{
		assert(!"ply greater than max ply");
		return evaluate();
	}

	int eval = evaluate();


	//fails high
	if (eval >= beta)
		return beta;
	//better move
	if (eval > alpha)
	{
		//PV node (Prinsipal variation)
		alpha = eval;
	}


	Moves moves[1];
	generate_moves(moves);

	sort_moves(moves);//sort!

	for (int i = 0; i < moves->count; i++)
	{
		if (!get_move_capture(moves->moves[i]))
			continue;


		copy_board();

		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		if (!make_move(moves->moves[i]))
		{
			take_back();

			ply--;
			repetition_index--;
			continue;
		}


		int score = -quiescence(-beta, -alpha);

		ply--;
		repetition_index--;
		take_back();
		//return if time is up
		if (stopped) return 0;

		//better move
		if (score > alpha)
		{
			//PV node (Prinsipal variation)
			alpha = score;

			//fails high
			if (score >= beta)
				return beta;
		}
	}
	//fails low
	return alpha;
}


const int full_depth_moves = 4;
const int reduction_limit = 3;

//main search
static inline int negamax(int alpha, int beta, int depth)
{
	//init pv length
	pv_length[ply] = ply;

	//current move score
	int score;
	int hash_flag = hash_flag_alpha;


	//return draw on repetition
	if (ply && is_repetition())
		return 0;


	//a hack to check if current node is pv or not 
	bool pv_node = beta - alpha > 1;

	//if move already searched return move score without further search
	//also make sure we are noot in the root ply and current node is not a PV node
	if (ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && !pv_node)
		return score;


	//every time to listen to gui / user
	if ((nodes & LISTEN) == 0)
		communicate();


	if (depth == 0)
		//run quisuince
		return quiescence(alpha, beta);

	//make sure no overflow on arrays
	if (ply > MAX_PLY - 1)
	{
		assert(!"ply greater than max ply");
		return evaluate();
	}

	//number of nodes
	nodes++;

	int in_check = is_square_attacked(
		(side == WHITE ? get_lsb_index(bitboards[K]) :
			get_lsb_index(bitboards[k])), side ^ 1);


	if (in_check) depth++;//search further when in check

	//legal moves counter
	int legal_count = 0;


	//Null move pruning
	if (depth >= 3 && !in_check && ply)
	{
		copy_board();

		//increment ply
		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		//hash enpassant if available
		if (enpassant != NO_SQ) hash_key ^= enpassant_keys[enpassant];

		//reset enpassant
		enpassant = NO_SQ;

		//switch side: giving opponent extra move
		side ^= 1;

		//hash side
		hash_key ^= side_key;

		//search with reduced depth (depth - 1 - r  where r = reduction_limit)
		score = -negamax(-beta, -beta + 1, depth - 1 - 2);

		//revert ply
		ply--;
		repetition_index--;

		take_back();

		//return if time is up
		if (stopped) return 0;

		//fail hard cutoff
		if (score >= beta)
			return beta;//fail high

	}

	Moves moves[1];
	generate_moves(moves);

	//we are now following pv line
	if (follow_pv)
		enable_pv_scoring(moves);

	sort_moves(moves);//sort!

	int moves_searched = 0;

	for (int i = 0; i < moves->count; i++)
	{
		copy_board();

		ply++;
		//add hash to repetition table & increment index
		repetition_table[++repetition_index] = hash_key;

		if (!make_move(moves->moves[i]))
		{
			take_back();
			ply--;
			repetition_index--;

			continue;
		}
		legal_count++;


		//LMR

		//full depth search
		if (moves_searched == 0)
			score = -negamax(-beta, -alpha, depth - 1);
		else//late move reduction LMR
		{
			//condition for LMR
			if (
				moves_searched >= full_depth_moves &&
				depth >= reduction_limit &&
				!in_check &&
				!get_move_capture(moves->moves[i]) &&
				!get_move_promoted(moves->moves[i])
				)
			{
				//search current move with reduced depth
				score = -negamax(-alpha - 1, -alpha, depth - 2);
			}
			else
			{
				//hack to ensure full depth search
				score = alpha + 1;
			}
			//principle variation search PVS
			if (score > alpha)
			{
				score = -negamax(-alpha - 1, -alpha, depth - 1);

				//if LMR fails re-search at full depth
				if (score > alpha && score < beta)
					score = -negamax(-beta, -alpha, depth - 1);
			}

		}

		ply--;
		repetition_index--;
		take_back();
		//return if time is up
		if (stopped) return 0;

		moves_searched++;

		//better move
		if (score > alpha)
		{
			//switch hash flag from fail low to one for PV node
			hash_flag = hash_flag_exact;


			//only quite moves
			if (!get_move_capture(moves->moves[i]))
			{
				//store history moves
				history_moves[get_move_piece(moves->moves[i])][get_move_target(moves->moves[i])] += depth;
			}

			//PV node (Prinsipal variation)
			alpha = score;

			//write PV move
			pv_table[ply][ply] = moves->moves[i];


			//loop over next ply
			for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
			{
				//copy move from deeper ply into curent
				pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
			}

			//adjust pv length
			pv_length[ply] = pv_length[ply + 1];


			//fails high (hard)
			if (score >= beta)
			{
				//store hash entry with score equal to beta
				write_hash_entry(beta, depth, hash_flag_beta);

				//only quite moves
				if (!get_move_capture(moves->moves[i]))
				{
					//store killer
					killer_moves[1][ply] = killer_moves[0][ply];
					killer_moves[0][ply] = moves->moves[i];
				}

				return beta;
			}
		}
	}
	//no legal moves
	if (legal_count == 0)
	{
		if (in_check)
			return -mate_value + ply;
		else//stalemate
			return 0;
	}

	//store hash entry with the score equal to alpha
	write_hash_entry(alpha, depth, hash_flag);

	//fails low
	return alpha;
}

void search_position(int depth)
{
	int previous_best = 0;
	int score = 0;

	//nodes = running total
	nodes = 0ULL;

	//reset stop flag
	stopped = false;

	//reset flags
	follow_pv = FALSE;
	score_pv = 0;

	//clear arrays
	memset(killer_moves, 0, sizeof(killer_moves));
	memset(history_moves, 0, sizeof(history_moves));
	memset(pv_table, 0, sizeof(pv_table));
	memset(pv_length, 0, sizeof(pv_length));


	//1281490
	 //770190

	//845015
	// 
	//depth 6 tricky position
	// 
	//pre best:	1056080
	//new best:  824253
	//new best:  363479

	int alpha = -infinity;
	int beta = infinity;

	//iterative deepening
	for (int current_depth = 1; current_depth <= depth; current_depth++)
	{
		//enable pv
		follow_pv = TRUE;

		//search best
		score = negamax(alpha, beta, current_depth);

		//we fell outside the window, try again with full window (and same depth)
		if ((score <= alpha) || (score >= beta))
		{
			alpha = -infinity;
			beta = infinity;
            --current_depth;
			continue;
		}

		//set up the window for the next itteration
		alpha = score - 50;
		beta = score + 50;

		//get best move so far if time is up
		if (stopped) break;
		previous_best = pv_table[0][0];

		//send info to gui
		std::cout << "info ";

		if (score > -mate_value && score < -mate_score)
            std::cout << "score mate " << -(score + mate_value) / 2;
		else if (score > mate_score && score < mate_value)
            std::cout << "score mate " << (mate_value - score) / 2;
		else
            std::cout << "score cp " << score;

        std::cout <<
        " depth " <<current_depth <<
        " nodes " << nodes <<
        " time " << (GetTickCount64() - start_time) <<
        " pv ";

		//loop over pv line
		for (int i = 0; i < pv_length[0]; i++)
            std::cout << get_move_string(pv_table[0][i]) << " ";
		std::cout << custom_endl;

		if (time_set && GetTickCount64() + 4 > stop_time)
		{
			//stopped = true; 	

			if (ply != 0)
				std::cout << "Ply err! ply->" << ply << custom_endl;

			assert(ply == 0);

			break;
		}
	}
	
	std::string m;
	m += "bestmove ";
	//if the search was stopped, and we have a previous best move
	if (stopped && previous_best)
		m += get_move_string(previous_best);
	//else if the search was not stopped, or we don't have a previous best move return current
	else
		m += get_move_string(pv_table[0][0]);

	std::cout << m << custom_endl;
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
	if (length < 4)
		throw std::invalid_argument("move string too short!");

	Moves move_list[1];

	generate_moves(move_list);

	//parse source        //x                      //y
	int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
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

			repetition_index++;
			repetition_table[repetition_index] = hash_key;

			make_move(move);

			//move the pointer to next move
			while (*current && *current != ' ') ++current;

			current++;//skip space
		}
	}
	//print_board(true);
}

static inline int get_piece_count()
{
	int cnt = 0;

	for (int i = 0; i < 12; i++)
		cnt += count_bits(bitboards[i]);

	return cnt;
}


//                pieces left   0, 1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
const int moves_by_pieces[] = { 0, 1, 3, 4, 5, 6, 8, 9, 10, 11, 13, 14, 15, 16, 18, 19, 20, 21, 23, 24, 25, 26, 28, 29, 30, 31, 33, 34, 35, 36, 38, 39, 40, 41, 43, 44, 45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 61, 63, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 76, 78, 79, 80 };

// parse UCI command "go"
void parse_go(const char* command)
{
	// init parameters

	//int moves_to_go = 30;

	int moves_to_go = moves_by_pieces[get_piece_count()];

	int depth = 0;
	U64 time = 0ULL, move_time = 0ULL, inc = 0ULL;
	const char* argument = NULL;

	time_set = false;

	// infinite search
	if ((argument = strstr(command, "infinite"))) {}

	// match UCI "binc" command
	if ((argument = strstr(command, "binc")) && side == BLACK)
	{
		// parse black time increment
		inc = atoi(argument + 5);
		time_set = true;
	}

	// match UCI "winc" command
	if ((argument = strstr(command, "winc")) && side == WHITE)
	{
		// parse white time increment
		inc = atoi(argument + 5);
		time_set = true;
	}

	// match UCI "wtime" command
	if ((argument = strstr(command, "wtime")) && side == WHITE)
	{
		// parse white time limit
		time = atoi(argument + 6);
		time_set = true;
	}

	// match UCI "btime" command
	if ((argument = strstr(command, "btime")) && side == BLACK)
	{
		// parse black time limit
		time = atoi(argument + 6);
		time_set = true;
	}

	// match UCI "movestogo" command
	if ((argument = strstr(command, "movestogo")))
		// parse number of moves to go
		moves_to_go = atoi(argument + 10);

	// match UCI "movetime" command
	if ((argument = strstr(command, "movetime")))
	{
		// parse amount of time allowed to spend to make a move
		move_time = atoi(argument + 9);
		time_set = true;
	}

	// match UCI "depth" command
	if ((argument = strstr(command, "depth")))
		// parse search depth
		depth = atoi(argument + 6);

	// if move time is available
	if (move_time != 0)
	{
		// set time equal to move time
		time = move_time;

		// set moves to go to 1
		moves_to_go = 1;
	}

	// init start time
	start_time = GetTickCount64();

	// if time control is available
	if (time_set)
	{		
		// set up timing
		time /= moves_to_go;

		time += inc;

		if (time > 200) time -= 30;
		else if (time > 100) time -= 26;
		else if (time > 50) time -= 22;
		else if (time > 25) time -= 18;
		else if (time > 10) time -= 7;
		else if (time > 5) time -= 5;
		else if (time > 1) time -= 1;
		else time = 0;

		//use this instead(fixes loosing on time)? seems to only fail with 0 time and increment
		// treat increment as seconds per move when time is almost up
		//if (time < 1500 && inc && depth == 64) stoptime = starttime + inc - 50;

		stop_time = start_time + time;
	}

	// if depth is not available
	if (depth == 0)
		// set depth to max
		depth = MAX_PLY;

	// print debug info
    std::cout <<
    "Time:" << time <<
    " Start:" << start_time <<
    " Stop:" << stop_time <<
    " Depth:" << depth <<
    " Time-set:" << (time_set ? "True" : "False") <<
    " MovesToGo:" << moves_to_go <<
    custom_endl;

	// search position
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

	//user/GUI buffer
	char input[size];

	//engine info
    std::cout << "CrusherChess " << VERSION << " by Hanno Kruger" << custom_endl;

	//uci loop
	while (true)
	{
		//reset input
		memset(input, 0, sizeof(input));

		//skip loop cycle while nothing in stdin else populate input
		if (!fgets(input, size, stdin)) continue;

		//skip newline
		if (input[0] == '\n') continue;

		//UCI isready
		if (strncmp(input, "isready", 7) == 0)
		{
            std::cout << "readyok" << custom_endl;
			continue;
		}
		//UCI position		 
		else if (strncmp(input, "position", 8) == 0)
		{
			parse_position(input);

			//clear_hash_table();
		}
		//UCI new game
		else if (strncmp(input, "ucinewgame", 10) == 0)
		{
			parse_position("position startpos");

			clear_hash_table();
		}
		//UCI go
		else if (strncmp(input, "go", 2) == 0)
			parse_go(input);
		//UCI quit
		else if (strncmp(input, "quit", 4) == 0)
			break;
		//greetings
		else if (strncmp(input, "uci", 3) == 0)
		{
            std::cout << "id name CrusherChess " << VERSION << custom_endl;
            std::cout << "id author Hanno Kruger" << custom_endl;
            std::cout << "option name Hash type spin default 64 min 1 max " << MAX_HASH << custom_endl;
            std::cout << "uciok" << custom_endl;
		}
		else if (!strncmp(input, "setoption name Hash value ", 26))
		{
			int mb = 64;

			// init MB
			sscanf_s(input, "%*s %*s %*s %*s %d", &mb);

			// adjust MB if going beyond the allowed bounds
			if (mb < 1) mb = 1;
			if (mb > MAX_HASH) mb = MAX_HASH;

			// set hash table size in MB
            std::cout << "Set hash table size to " << mb << "MB" << custom_endl;
			init_hash_table(mb);
		}
		else if (input[0] == 'd')
			print_board();
		else
			std::cout <<"Unknown command" << custom_endl;

		//fflush(stdin);
	}
}


#pragma endregion
/****************************************\
 ========================================
				 Main
 ========================================
\****************************************/


//#define DEBUG

int main()
{ 
	//setvbuf(stdout, NULL, _IONBF, 0);
	//std::cout.setf(std::ios_base::unitbuf);

	//use %ls for wchar

	//need to fix score imbalance with mirror table eroor in this position "1k6/8/8/8/8/8/8/6K1 w - - 0 1"

	//bugs:
	//lose on time
	//null move from pv, is this fixed now?

	init_all();

#ifdef DEBUG
		//tricky
		//parse_fen("r3k2r/p1ppqpb1/1n2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R w KQkq - 0 1 ");
        parse_fen("6Br/PP2k2P/6K1/8/nn6/b1n3n1/pppppp1p/4b3 w - - 0 1");
		//parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");
		//parse_fen(start_position);

		print_board();

		printf("score:%d\n", evaluate());

        std::cout << "\nRunning perft" << custom_endl;
		perft_test(2);
		
		//auto startTime = std::chrono::high_resolution_clock::now();

		//search_position(10);
		//make_move(pv_table[0][0]);

		//search_position(10);
		//make_move(pv_table[0][0]);
	
		//search_position(10);
		//make_move(pv_table[0][0]);

		//search_position(10);
		//make_move(pv_table[0][0]);

		//print_board();

		//auto endTime = std::chrono::high_resolution_clock::now();

		//auto elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//auto elapsedMS = ((long double)elapsedMicro) / 1000;
		//auto elapsedS = elapsedMS / 1000;
	
		//printf("Total Time:%.2LF (MS)\n", elapsedMS);
		//printf("Seconds:%.2LF (S)\n\n", elapsedS);





	 //   startTime = std::chrono::high_resolution_clock::now();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();
		//
		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e6d5");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		////print_board();

		//parse_position("position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e6d5 c3d5");
		//clear_hash_table();
		//parse_go("go depth 10");
		//make_move(pv_table[0][0]);
		//print_board();


	 //   endTime = std::chrono::high_resolution_clock::now();

		//elapsedMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//elapsedMS = ((long double)elapsedMicro) / 1000;
		//elapsedS = elapsedMS / 1000;

		//printf("Total Time:%.2LF (MS)\n", elapsedMS);
		//printf("Seconds:%.2LF (S)\n\n", elapsedS);

		
		//getchar();

		std::wcin.get();
#else
		uci_loop();
#endif

	free(hash_table);
	return 0;
}