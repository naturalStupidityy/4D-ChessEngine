#pragma once
#include "types.h"
#include "state.h"
#include "search.h"
#include <string>
#include <vector>
#include <random>

class ChessEngine4D {
public:
    ChessEngine4D();

    // Initialize the engine with standard starting position
    void new_game();

    // Get all legal moves for the current side
    std::vector<Move> get_legal_moves() const;

    // Apply a move (player's move)
    bool apply_move(const Move& move);

    // Get the best move for the current position (AI)
    Move get_best_move(int depth = 5);

    // Check for cat/phantom events and apply them
    bool process_special_events();

    // Get current game state info
    bool is_game_over() const;
    bool is_in_check() const;
    Color side_to_move() const;

    // Convert move to/from string notation
    std::string move_to_string(const Move& move) const;
    Move string_to_move(const std::string& str) const;

    // Get board as string for display
    std::string board_to_string() const;

    // Access the underlying state
    const GameState& state() const { return state_; }
    GameState& state() { return state_; }

    // Export/import state (for Godot bridge)
    std::string export_state() const;
    bool import_state(const std::string& state_str);

    // Perft for testing
    uint64_t perft(int depth) const;

private:
    GameState state_;
    Searcher searcher_;
    std::mt19937 rng_;
};
