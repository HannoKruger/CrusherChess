#pragma once
int Mposition_evaluate(int cnt, int depth);
int MMiniMax(int depth, bool Maximizing);
int Mget_best_move(int depth);
void Msearch_position(int depth);

const int piece_scores[12] = { 10, 30, 31, 50, 90, 2000, -10, -30, -31, -50, -90, -2000 };

