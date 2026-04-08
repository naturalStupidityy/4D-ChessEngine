#include <gtest/gtest.h>
#include "types.h"
#include "board.h"
#include "zobrist.h"

TEST(ZobristTest, HashDeterministic) {
    Board b;
    b.setup_initial_position();
    ZobristHash z1, z2;
    EXPECT_EQ(z1.compute_hash(b, Color::WHITE),
              z2.compute_hash(b, Color::WHITE));
}

TEST(ZobristTest, HashDiffersForDifferentSide) {
    Board b;
    b.setup_initial_position();
    ZobristHash z;
    uint64_t w = z.compute_hash(b, Color::WHITE);
    uint64_t bl = z.compute_hash(b, Color::BLACK);
    EXPECT_NE(w, bl);
}

TEST(ZobristTest, HashDiffersAfterMove) {
    Board b;
    b.setup_initial_position();
    ZobristHash z;
    uint64_t h_before = z.compute_hash(b, Color::WHITE);

    // Make a simple move: move a pawn from w=1 to w=2
    int from = flat_index(0, 0, 0, 1);
    int to   = flat_index(0, 0, 0, 2);
    Piece pawn = b.at(from);
    b.remove_piece(from);
    b.set_piece(to, pawn);

    uint64_t h_after = z.compute_hash(b, Color::WHITE);
    EXPECT_NE(h_before, h_after);
}

TEST(ZobristTest, PieceHashNonZero) {
    ZobristHash z;
    for (int p = 1; p < 9; ++p) {
        for (int c = 0; c < 2; ++c) {
            uint64_t h = z.piece_hash(static_cast<PieceType>(p),
                                     static_cast<Color>(c), 0);
            EXPECT_NE(h, 0ULL);
        }
    }
}

TEST(ZobristTest, SideHashNonZero) {
    ZobristHash z;
    EXPECT_NE(z.side_hash(), 0ULL);
}

TEST(ZobristTest, EmptyBoardSameForBothSides) {
    Board b;
    b.clear();
    ZobristHash z;
    // With an empty board and ignoring side, hashes should differ only by side_key
    uint64_t w  = z.compute_hash(b, Color::WHITE);
    uint64_t bl = z.compute_hash(b, Color::BLACK);
    EXPECT_EQ(w ^ bl, z.side_hash());
}
