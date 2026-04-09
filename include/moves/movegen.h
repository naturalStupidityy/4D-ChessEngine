#pragma once
#include "types.h"
#include "board.h"
#include <vector>

class MoveGenerator {
public:
    MoveGenerator();

    // Generate all pseudo-legal moves (may leave king in check)
    int generate_pseudo_legal(const Board& board, Color side, Move* moves) const;

    // Generate only legal moves (filters out those leaving king in check)
    int generate_legal(const Board& board, Color side, Move* moves) const;

    // Count legal moves at depth d (perft)
    uint64_t perft(const Board& board, Color side, int depth) const;

    // Cat event: check if cat should spawn
    bool should_spawn_cat() const;

    // Get boundary cells for cat's ability
    std::vector<int> get_boundary_enemy_cells(const Board& board, Color cat_color) const;

    // Check if a given flat index is a boundary cell
    static bool is_boundary(int idx);

    // Check if position is in check for given color
    bool is_in_check(const Board& board, Color color) const;

    // Check if a move is legal (doesn't leave own king in check)
    bool is_legal_move(const Board& board, Color side, const Move& move) const;

private:
    // Generate moves for each piece type
    void gen_sliding(const Board& board, Color side, Move* moves, int& count,
                     const Coord4D* dirs, int num_dirs, int from_idx) const;
    void gen_stepping(const Board& board, Color side, Move* moves, int& count,
                      const Coord4D* dirs, int num_dirs, int from_idx) const;
    void gen_knight(const Board& board, Color side, Move* moves, int& count, int from_idx) const;
    void gen_king(const Board& board, Color side, Move* moves, int& count, int from_idx) const;
    void gen_pawn(const Board& board, Color side, Move* moves, int& count, int from_idx) const;
    void gen_phantom(const Board& board, Color side, Move* moves, int& count, int from_idx) const;

    // Check if a square is attacked by the given color
    bool is_square_attacked(const Board& board, int idx, Color attacker) const;

    // Precomputed direction tables
    static constexpr Coord4D ROOK_DIRS[8] = {
        {1,0,0,0},{-1,0,0,0},{0,1,0,0},{0,-1,0,0},
        {0,0,1,0},{0,0,-1,0},{0,0,0,1},{0,0,0,-1}
    };

    // All 24 face-diagonal directions
    static constexpr Coord4D BISHOP_DIRS[24] = {
        // x-y plane diagonals
        {1,1,0,0},{1,-1,0,0},{-1,1,0,0},{-1,-1,0,0},
        // x-z plane diagonals
        {1,0,1,0},{1,0,-1,0},{-1,0,1,0},{-1,0,-1,0},
        // x-w plane diagonals
        {1,0,0,1},{1,0,0,-1},{-1,0,0,1},{-1,0,0,-1},
        // y-z plane diagonals
        {0,1,1,0},{0,1,-1,0},{0,-1,1,0},{0,-1,-1,0},
        // y-w plane diagonals
        {0,1,0,1},{0,1,0,-1},{0,-1,0,1},{0,-1,0,-1},
        // z-w plane diagonals
        {0,0,1,1},{0,0,1,-1},{0,0,-1,1},{0,0,-1,-1}
    };

    static constexpr Coord4D QUEEN_DIRS[32] = {
        // Rook dirs (8)
        {1,0,0,0},{-1,0,0,0},{0,1,0,0},{0,-1,0,0},
        {0,0,1,0},{0,0,-1,0},{0,0,0,1},{0,0,0,-1},
        // Bishop dirs (24)
        {1,1,0,0},{1,-1,0,0},{-1,1,0,0},{-1,-1,0,0},
        {1,0,1,0},{1,0,-1,0},{-1,0,1,0},{-1,0,-1,0},
        {1,0,0,1},{1,0,0,-1},{-1,0,0,1},{-1,0,0,-1},
        {0,1,1,0},{0,1,-1,0},{0,-1,1,0},{0,-1,-1,0},
        {0,1,0,1},{0,1,0,-1},{0,-1,0,1},{0,-1,0,-1},
        {0,0,1,1},{0,0,1,-1},{0,0,-1,1},{0,0,-1,-1}
    };

    // Knight vectors: choose 2 axes, one changes +-1, other +-2
    // C(4,2)=6 pairs x 2 assignments x 4 sign combos = 48
    static constexpr Coord4D KNIGHT_DIRS[48] = {
        // x-y pair: x+-1,y+-2 and x+-2,y+-1
        {1,2,0,0},{1,-2,0,0},{-1,2,0,0},{-1,-2,0,0},{2,1,0,0},{2,-1,0,0},{-2,1,0,0},{-2,-1,0,0},
        // x-z pair
        {1,0,2,0},{1,0,-2,0},{-1,0,2,0},{-1,0,-2,0},{2,0,1,0},{2,0,-1,0},{-2,0,1,0},{-2,0,-1,0},
        // x-w pair
        {1,0,0,2},{1,0,0,-2},{-1,0,0,2},{-1,0,0,-2},{2,0,0,1},{2,0,0,-1},{-2,0,0,1},{-2,0,0,-1},
        // y-z pair
        {0,1,2,0},{0,1,-2,0},{0,-1,2,0},{0,-1,-2,0},{0,2,1,0},{0,2,-1,0},{0,-2,1,0},{0,-2,-1,0},
        // y-w pair
        {0,1,0,2},{0,1,0,-2},{0,-1,0,2},{0,-1,0,-2},{0,2,0,1},{0,2,0,-1},{0,-2,0,1},{0,-2,0,-1},
        // z-w pair
        {0,0,1,2},{0,0,1,-2},{0,0,-1,2},{0,0,-1,-2},{0,0,2,1},{0,0,2,-1},{0,0,-2,1},{0,0,-2,-1}
    };

    // King directions: all 80 Chebyshev distance-1 vectors in 4D
    static constexpr int NUM_KING_DIRS = 80;
    mutable Coord4D king_dirs_[80];
    mutable bool king_dirs_initialized_ = false;
    void init_king_dirs() const;
};
