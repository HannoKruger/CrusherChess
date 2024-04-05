#include <cstring>
#include "movegen.h"
#include "bitoperations.h"
#include "zobristKeys.h"
#include "magics.h"


/****************************************\
 ========================================
			Move Generator
 ========================================
\****************************************/
#pragma region Move Generator




//version 2

//96 + 24 + 4 + 4 = 128
//=> 128 * depth

//char* boardstates = new char[128*60];

//long index = 0;

//static  void copy_board()
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



//static  void take_back()
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



//static  void copy_board()
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
//static  void take_back()
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
 bool is_square_attacked(int square, int side)
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

 void add_move(Moves* move_list, int move)
{
	//store move
	move_list->moves[move_list->count] = move;

	move_list->count++;
}

//generates all moves
 void generate_moves(Moves *move_list)
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

 int make_move(int move)
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