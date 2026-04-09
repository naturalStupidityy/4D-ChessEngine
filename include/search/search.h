#pragma once
#include "types.h"
#include "state.h"
#include "eval.h"
#include <vector>
#include <unordered_map>

enum class NodeType : uint8_t { EXACT, LOWER_BOUND, UPPER_BOUND };

struct TTEntry {
    uint64_t hash = 0;
    int16_t score = 0;
    int8_t depth = 0;
    Move best_move;
    NodeType type = NodeType::EXACT;
    uint8_t age = 0;
};

class Searcher {
public:
    Searcher();

    // Find the best move for the current position
    Move find_best_move(GameState& state, int max_depth = 5);

    // Get the evaluation score of the current position
    int evaluate_position(GameState& state, int depth = 5);

    void set_max_depth(int depth) { max_depth_ = depth; }
    int get_max_depth() const { return max_depth_; }
    uint64_t nodes_searched() const { return nodes_searched_; }

    size_t tt_size() const { return transposition_table_.size(); }
    void clear_tt() { transposition_table_.clear(); }

    // Order moves for better pruning
    void order_moves(Move* moves, int count, const GameState& state) const;

private:
    // Alpha-beta negamax search
    int alpha_beta(GameState& state, int depth, int alpha, int beta);

    // Quiescence search (capture-only to avoid horizon effect)
    int quiescence(GameState& state, int alpha, int beta);

    int max_depth_;
    uint64_t nodes_searched_;

    Evaluator evaluator_;

    // Transposition table
    static constexpr size_t TT_CAPACITY = 1 << 20; // 1M entries
    std::unordered_map<uint64_t, TTEntry> transposition_table_;

    uint8_t current_age_ = 0;
};
