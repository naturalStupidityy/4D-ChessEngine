#include "zobrist.h"
#include <cstdint>

ZobristHash::ZobristHash() {
    seed(0xA5A5A5A5A5A5A5A5ULL);
}

void ZobristHash::seed(uint64_t s) {
    // Split 64-bit seed into two 32-bit seeds for the Mersenne Twister
    std::mt19937_64 rng(s);

    for (int p = 0; p < 9; ++p) {
        for (int c = 0; c < 3; ++c) {
            for (int i = 0; i < 256; ++i) {
                table_[p][c][i] = rng();
            }
        }
    }
    side_key_ = rng();
}

uint64_t ZobristHash::piece_hash(PieceType type, Color color, int flat_idx) const {
    return table_[static_cast<int>(type)][static_cast<int>(color)][flat_idx];
}

uint64_t ZobristHash::side_hash() const {
    return side_key_;
}

uint64_t ZobristHash::compute_hash(const Board& board, Color side_to_move) const {
    uint64_t h = 0;

    for (int i = 0; i < NUM_CELLS; ++i) {
        const Piece& p = board.at(i);
        if (!p.is_none()) {
            h ^= piece_hash(p.type, p.color, i);
        }
    }

    if (side_to_move == Color::BLACK) {
        h ^= side_key_;
    }

    return h;
}
