#pragma once
#include "types.h"
#include "board.h"
#include <array>

class Evaluator {
public:
    Evaluator();

    // Evaluate position from the perspective of the given color
    // Positive = good for the color
    int evaluate(const Board& board, Color side) const;

    // Material values for each piece type
    static constexpr int MATERIAL_VALUES[9] = {
        0,     // NONE
        100,   // PAWN
        320,   // KNIGHT
        330,   // BISHOP
        500,   // ROOK
        900,   // QUEEN
        20000, // KING
        150,   // CAT (volatile, lower value)
        200    // PHANTOM
    };

    // Centrality bonus based on distance to hyper-center
    int centrality_bonus(int flat_idx) const;

private:
    // Piece-square tables for each piece type (256 entries)
    std::array<int, 256> pst_pawn_;
    std::array<int, 256> pst_knight_;
    std::array<int, 256> pst_bishop_;
    std::array<int, 256> pst_rook_;
    std::array<int, 256> pst_queen_;
    std::array<int, 256> pst_king_;

    void init_pst(std::array<int, 256>& table, int center_bonus);

    // Compute hyper-centre control score
    int hyper_centre_control(const Board& board, Color color) const;

    // Compute king safety
    int king_safety(const Board& board, Color color) const;

    // Get the piece-square table for a given piece type
    const std::array<int, 256>* get_pst(PieceType type) const;
};
