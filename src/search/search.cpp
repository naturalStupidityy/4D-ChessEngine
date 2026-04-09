#include "search.h"
#include <algorithm>
#include <climits>

Searcher::Searcher()
    : max_depth_(5),
      nodes_searched_(0),
      current_age_(0) {}

// --- Iterative deepening ----------------------------------------------------------
Move Searcher::find_best_move(GameState& state, int max_depth) {
    max_depth_ = max_depth;
    nodes_searched_ = 0;
    ++current_age_;

    Move best_move;

    // Iterative deepening from 1 to max_depth
    for (int depth = 1; depth <= max_depth_; ++depth) {
        alpha_beta(state, depth, -999999, 999999);

        // Retrieve the best move from the TT
        auto it = transposition_table_.find(state.hash());
        if (it != transposition_table_.end() &&
            it->second.best_move.from != it->second.best_move.to) {
            best_move = it->second.best_move;
        }
    }

    return best_move;
}

int Searcher::evaluate_position(GameState& state, int depth) {
    max_depth_ = depth;
    nodes_searched_ = 0;
    return alpha_beta(state, depth, -999999, 999999);
}

// --- Alpha-beta negamax -----------------------------------------------------------
int Searcher::alpha_beta(GameState& state, int depth, int alpha, int beta) {
    ++nodes_searched_;

    // Terminal / leaf
    if (depth == 0) {
        return quiescence(state, alpha, beta);
    }

    if (state.is_game_over()) {
        if (state.is_checkmate()) {
            // The side to move is checkmated — very bad for them
            return -999999 - depth;  // prefer faster mates
        }
        return 0; // stalemate
    }

    uint64_t h = state.hash();

    // --- Transposition table lookup ---
    auto it = transposition_table_.find(h);
    if (it != transposition_table_.end() && it->second.depth >= depth) {
        const TTEntry& entry = it->second;
        int tt_score = entry.score;

        if (entry.type == NodeType::EXACT) return tt_score;
        if (entry.type == NodeType::LOWER_BOUND && tt_score > alpha) alpha = tt_score;
        if (entry.type == NodeType::UPPER_BOUND && tt_score < beta)  beta  = tt_score;
        if (alpha >= beta) return tt_score;
    }

    Move moves[MAX_MOVES];
    int count = state.generate_legal_moves(moves);

    // Order moves
    order_moves(moves, count, state);

    int best_score = -999999;
    Move local_best_move;

    for (int i = 0; i < count; ++i) {
        state.make_move(moves[i]);
        int score = -alpha_beta(state, depth - 1, -beta, -alpha);
        state.unmake_move(moves[i]);

        if (score > best_score) {
            best_score = score;
            local_best_move = moves[i];
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // beta cutoff
    }

    // --- Store in TT ---
    TTEntry entry;
    entry.hash = h;
    entry.score = static_cast<int16_t>(best_score);
    entry.depth = static_cast<int8_t>(depth);
    entry.best_move = local_best_move;
    entry.age = current_age_;

    if (best_score <= alpha)
        entry.type = NodeType::UPPER_BOUND;
    else if (best_score >= beta)
        entry.type = NodeType::LOWER_BOUND;
    else
        entry.type = NodeType::EXACT;

    transposition_table_[h] = entry;

    return best_score;
}

// --- Quiescence search ------------------------------------------------------------
int Searcher::quiescence(GameState& state, int alpha, int beta) {
    ++nodes_searched_;

    if (state.is_game_over()) {
        if (state.is_checkmate()) return -999999;
        return 0;
    }

    int stand_pat = evaluator_.evaluate(state.board(), state.side_to_move());

    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    // Generate only capture moves
    Move all_moves[MAX_MOVES];
    int count = state.generate_legal_moves(all_moves);

    for (int i = 0; i < count; ++i) {
        if (!all_moves[i].is_capture()) continue;

        state.make_move(all_moves[i]);
        int score = -quiescence(state, -beta, -alpha);
        state.unmake_move(all_moves[i]);

        if (score > alpha) alpha = score;
        if (alpha >= beta) return beta;
    }

    return alpha;
}

// --- Move ordering ----------------------------------------------------------------
void Searcher::order_moves(Move* moves, int count, const GameState& state) const {
    // We score each move for ordering
    // Priority: TT best move >> captures (MVV-LVA) >> quiet moves
    int scores[MAX_MOVES];

    // Look up TT best move
    Move tt_best;
    auto tt_it = transposition_table_.find(state.hash());
    if (tt_it != transposition_table_.end()) tt_best = tt_it->second.best_move;

    for (int i = 0; i < count; ++i) {
        scores[i] = 0;

        // TT best move gets highest priority
        if (moves[i].from == tt_best.from && moves[i].to == tt_best.to) {
            scores[i] = 1000000;
            continue;
        }

        if (moves[i].is_capture()) {
            // MVV-LVA: value of captured piece - value of capturing piece
            int victim_val = Evaluator::MATERIAL_VALUES[static_cast<int>(moves[i].captured)];
            const Piece& attacker = state.board().at(moves[i].from);
            int attacker_val = Evaluator::MATERIAL_VALUES[static_cast<int>(attacker.type)];
            scores[i] = 100000 + victim_val - attacker_val / 100;

            if (moves[i].is_promotion()) {
                scores[i] += Evaluator::MATERIAL_VALUES[static_cast<int>(moves[i].promotion)];
            }
        } else if (moves[i].is_promotion()) {
            scores[i] = 90000 + Evaluator::MATERIAL_VALUES[static_cast<int>(moves[i].promotion)];
        }
    }

    // Simple insertion sort (small arrays)
    for (int i = 1; i < count; ++i) {
        int key_score = scores[i];
        Move key_move = moves[i];
        int j = i - 1;
        while (j >= 0 && scores[j] < key_score) {
            scores[j + 1] = scores[j];
            moves[j + 1] = moves[j];
            --j;
        }
        scores[j + 1] = key_score;
        moves[j + 1] = key_move;
    }
}
