#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

struct Move;
struct State;

enum Flag { EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    Flag flag;
};

struct ScoredMove {
    Move move;
    int score;
};

extern std::unordered_map<uint64_t, TTEntry> g_transposition_table;
extern uint64_t g_nodes_searched;

int score_move(const Move& move);
std::vector<ScoredMove> order_moves(const std::vector<Move>& moves);
void clear_transposition_table();
int search(State& S, int depth, int alpha, int beta);
