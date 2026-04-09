#pragma once
#include "types.h"
#include "board.h"
#include <cstdint>
#include <random>

class ZobristHash {
public:
    ZobristHash();

    uint64_t compute_hash(const Board& board, Color side_to_move) const;
    uint64_t piece_hash(PieceType type, Color color, int flat_idx) const;
    uint64_t side_hash() const;
    void seed(uint64_t s);

private:
    uint64_t table_[9][3][256]; // [piece_type][color][cell]
    uint64_t side_key_;
};
