#include "search/search.h"

#include <algorithm>
#include <limits>

// Engine-provided functions.
std::vector<Move> generate_legal_moves(State& state);
void make_move(State& state, const Move& move);
void unmake_move(State& state, const Move& move);
int evaluate(State& state);
uint64_t get_zobrist_hash(const State& state);

std::unordered_map<uint64_t, TTEntry> g_transposition_table;
uint64_t g_nodes_searched = 0;

int score_move(const Move& move) {
    int score = 0;
    if (move.is_capture) {
        score += 100000;
    }
    if (move.is_promotion) {
        score += 50000;
    }
    if (move.is_check) {
        score += 10000;
    }
    return score;
}

std::vector<ScoredMove> order_moves(const std::vector<Move>& moves) {
    std::vector<ScoredMove> scored_moves;
    scored_moves.reserve(moves.size());

    for (const Move& move : moves) {
        scored_moves.push_back({move, score_move(move)});
    }

    std::sort(
        scored_moves.begin(),
        scored_moves.end(),
        [](const ScoredMove& a, const ScoredMove& b) {
            return a.score > b.score;
        }
    );

    return scored_moves;
}

void clear_transposition_table() {
    g_transposition_table.clear();
    g_nodes_searched = 0;
}

int search(State& S, int depth, int alpha, int beta) {
    ++g_nodes_searched;

    const int original_alpha = alpha;
    const int original_beta = beta;
    const uint64_t hash = get_zobrist_hash(S);

    const auto tt_it = g_transposition_table.find(hash);
    if (tt_it != g_transposition_table.end()) {
        const TTEntry& entry = tt_it->second;
        if (entry.depth >= depth) {
            if (entry.flag == EXACT) {
                return entry.score;
            }
            if (entry.flag == LOWERBOUND) {
                alpha = std::max(alpha, entry.score);
            } else if (entry.flag == UPPERBOUND) {
                beta = std::min(beta, entry.score);
            }
            if (alpha >= beta) {
                return entry.score;
            }
        }
    }

    if (depth == 0) {
        return evaluate(S);
    }

    const std::vector<Move> moves = generate_legal_moves(S);
    if (moves.empty()) {
        return evaluate(S);
    }

    const std::vector<ScoredMove> ordered_moves = order_moves(moves);

    int best_score = std::numeric_limits<int>::min();

    for (const ScoredMove& scored_move : ordered_moves) {
        const Move& move = scored_move.move;

        make_move(S, move);
        const int score = -search(S, depth - 1, -beta, -alpha);
        unmake_move(S, move);

        if (score > best_score) {
            best_score = score;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }

    TTEntry new_entry{};
    new_entry.hash = hash;
    new_entry.depth = depth;
    new_entry.score = best_score;

    if (best_score <= original_alpha) {
        new_entry.flag = UPPERBOUND;
    } else if (best_score >= original_beta) {
        new_entry.flag = LOWERBOUND;
    } else {
        new_entry.flag = EXACT;
    }

    g_transposition_table[hash] = new_entry;
    return best_score;
}
