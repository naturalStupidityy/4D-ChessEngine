#pragma once
#include "types.h"
#include "board.h"
#include "zobrist.h"
#include "movegen.h"
#include <vector>
#include <stack>
#include <random>

struct UndoInfo {
    Piece captured;
    MoveFlags flags;
    uint8_t from;
    uint8_t to;
    uint64_t hash;
    PieceType promotion;
    int halfmove_clock;
    uint8_t castling_rights;
    uint8_t en_passant_target;
    // For cat/phantom events
    std::vector<std::pair<int, Piece>> removed_pieces;
    std::vector<std::pair<int, Piece>> spawned_pieces;
};

class GameState {
public:
    GameState();

    // Initialize with standard starting position
    void setup_initial();

    // Make a move on the board
    void make_move(const Move& move);

    // Unmake the last move
    void unmake_move(const Move& move);

    // Generate legal moves for current side
    int generate_legal_moves(Move* moves) const;

    // Check if current side is in check
    bool in_check() const;

    // Check if game is over (checkmate or stalemate)
    bool is_game_over() const;

    // Check if it's checkmate
    bool is_checkmate() const;

    // Check if it's stalemate
    bool is_stalemate() const;

    // Trigger cat event (random spawn + boundary clear)
    bool trigger_cat_event(std::mt19937& rng);

    // Trigger phantom event
    bool trigger_phantom_event(std::mt19937& rng);

    // Accessors
    const Board& board() const { return board_; }
    Board& board() { return board_; }
    Color side_to_move() const { return side_to_move_; }
    void set_side_to_move(Color c) { side_to_move_ = c; }
    uint64_t hash() const { return current_hash_; }
    void recompute_hash() { compute_hash(); }
    int halfmove_clock() const { return halfmove_clock_; }
    int fullmove_number() const { return fullmove_number_; }

    const ZobristHash& zobrist() const { return zobrist_; }
    const MoveGenerator& movegen() const { return movegen_; }

private:
    Board board_;
    Color side_to_move_;
    uint64_t current_hash_;
    int halfmove_clock_;
    int fullmove_number_;

    uint8_t castling_rights_;
    uint8_t en_passant_target_;

    std::stack<UndoInfo> undo_stack_;

    ZobristHash zobrist_;
    MoveGenerator movegen_;

    void compute_hash();
};
