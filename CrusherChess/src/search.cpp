#include <cassert>
#include <cstring>
#include <sysinfoapi.h>
#include "search.h"
#include "uci.h"
#include "evaluation.h"
#include "movegen.h"
#include "transposition.h"
#include "zobristkeys.h"


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


void enable_pv_scoring(Moves* move_list)
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

int score_move(int move)
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

void sort_moves(Moves* move_list)
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
bool is_repetition()
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
int quiescence(int alpha, int beta)
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
int negamax(int alpha, int beta, int depth)
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

