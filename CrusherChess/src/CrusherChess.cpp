#include <cassert>
#include "types.h"
#include "magics.h"
#include "transposition.h"
#include "zobristKeys.h"
#include "uci.h"
#include "perft.h"
#include "evaluation.h"

using namespace CrusherChess;


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

void printArch() {
    #if defined(__x86_64__) || defined(_M_X64)
    // 64-bit architecture specific code
    #if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
        std::cout << "Compiled for 64-bit Windows." << std::endl;
    #elif defined(__APPLE__)
        std::cout << "Compiled for 64-bit macOS." << std::endl;
    #define MACOS
    #elif defined(__linux__)
        std::cout << "Compiled for 64-bit Linux." << std::endl;
    #define LINUX
    #else
        std::cout << "Compiled for 64-bit on an unknown platform." << std::endl;
    #endif
    #elif defined(__i386) || defined(_M_IX86)
    // 32-bit architecture specific code
    #if defined(_WIN32) || defined(_WIN64)
        std::cout << "Compiled for 32-bit Windows." << std::endl;
    #define WINDOWS
    #elif defined(__APPLE__)
        std::cout << "Compiled for 32-bit macOS." << std::endl;
    #define MACOS
    #elif defined(__linux__)
        std::cout << "Compiled for 32-bit Linux." << std::endl;
    #define LINUX
    #else
        std::cout << "Compiled for 32-bit on an unknown platform." << std::endl;
    #endif
    #else
    std::cout << "Compiled for an unknown architecture and platform." << std::endl;
    #endif
}


//#define DEBUG

int main()
{
    printArch();

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
        //parse_fen("6Br/PP2k2P/6K1/8/nn6/b1n3n1/pppppp1p/4b3 w - - 0 1");
		//parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");
		parse_fen(start_position);

		print_board();

		printf("score:%d\n", evaluate());

        std::cout << "\nRunning perft" << custom_endl;
		perft_test(4);
		
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