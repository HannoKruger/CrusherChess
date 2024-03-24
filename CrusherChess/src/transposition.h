#pragma once

//score layout
//[-inf...-mate_value...-mate_score...normal(non mate)score...mate_score...mate_value...inf]


#include "typedefs.h"
#define infinity 50000
#define mate_value 49000
#define mate_score 48000

// hash table size (16mb)
//#define tt_size 16
extern U64 hash_entries;

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
extern TT* hash_table;

void init_hash_table(int mb);
void clear_hash_table();
int read_hash_entry(int alpha, int beta, int depth);
void write_hash_entry(int score, int depth, int hash_flag);