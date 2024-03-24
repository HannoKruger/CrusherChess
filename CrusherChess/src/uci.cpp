#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <sysinfoapi.h>

#include <wincon.h>
#include <io.h>
#include <cassert>
#include <winbase.h>

#include "uci.h"
#include "transposition.h"
#include "hash.h"
#include "movegen.h"



bool quit = false; // exit engine flag
bool time_set = false; // time control availability
bool stopped = false; // when time is up
U64 start_time = 0ULL; // uci starttime command
U64 stop_time = 0ULL; // uci stoptime command





/****************************************\
 ========================================
			  Time control
 ========================================
\****************************************/


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
void communicate()
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

int get_piece_count()
{
	int cnt = 0;

	for (int i = 0; i < 12; i++)
		cnt += count_bits(bitboards[i]);

	return cnt;
}


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


    throw std::runtime_error("Not implemented");
	// search position
	//search_position(depth);
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
		else if (input[0] == 'd'){
            throw new std::runtime_error("Not implemented");
			//print_board();
            }
		else
			std::cout <<"Unknown command" << custom_endl;

		//fflush(stdin);
	}
}

