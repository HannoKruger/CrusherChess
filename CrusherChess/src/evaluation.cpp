#include "evaluation.h"
#include "magics.h"


int get_game_phase_score()
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

int lerp_positional_score(int piece_type,int square,int game_phase_score)
{
	return
		(
		positional_score[OPENING][piece_type][square] * game_phase_score +
		positional_score[ENDGAME][piece_type][square] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}
int lerp_material_score(int piece, int game_phase_score)
{
	return
		(material_score[OPENING][piece] * game_phase_score
		+ material_score[ENDGAME][piece] * (opening_phase_score - game_phase_score)
		) / opening_phase_score;
}


int evaluate()
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
