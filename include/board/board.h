#pragma once
#include "types.h"
#include <array>

class Board {
public:
    Board();

    void clear();
    void set_piece(int flat_idx, const Piece& piece);
    void set_piece(const Coord4D& c, const Piece& piece);
    void remove_piece(int flat_idx);
    void remove_piece(const Coord4D& c);

    const Piece& at(int flat_idx) const { return cells_[flat_idx]; }
    const Piece& at(const Coord4D& c) const { return cells_[flat_index(c)]; }
    Piece& at(int flat_idx) { return cells_[flat_idx]; }
    Piece& at(const Coord4D& c) { return cells_[flat_index(c)]; }

    int find_king(Color color) const;

    void setup_initial_position();
    bool is_boundary(int flat_idx) const;

    const std::array<Piece, NUM_CELLS>& cells() const { return cells_; }
    std::array<Piece, NUM_CELLS>& cells() { return cells_; }

    static constexpr bool valid_index(int idx) { return idx >= 0 && idx < NUM_CELLS; }

private:
    std::array<Piece, NUM_CELLS> cells_;
};
