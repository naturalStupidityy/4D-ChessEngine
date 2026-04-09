#include <gtest/gtest.h>
#include "types.h"
#include "board.h"

// ---------- Flat-index bijection --------------------------------------------------

TEST(BijectionTest, FlatIndexRoundTrip) {
    // phi(phi_inv(i)) = i  for all 256 indices
    for (int i = 0; i < NUM_CELLS; ++i) {
        Coord4D c = coord_4d(i);
        int back = flat_index(c);
        ASSERT_EQ(back, i) << "Failed round-trip for index " << i;
    }
}

TEST(BijectionTest, CoordRoundTrip) {
    // phi_inv(phi(x,y,z,w)) = (x,y,z,w)  for all valid coords
    for (int x = 0; x < 4; ++x)
        for (int y = 0; y < 4; ++y)
            for (int z = 0; z < 4; ++z)
                for (int w = 0; w < 4; ++w) {
                    int idx = flat_index(x, y, z, w);
                    Coord4D back = coord_4d(idx);
                    ASSERT_EQ(back.x, x);
                    ASSERT_EQ(back.y, y);
                    ASSERT_EQ(back.z, z);
                    ASSERT_EQ(back.w, w);
                }
}

TEST(BijectionTest, AllIndicesUnique) {
    std::set<int> seen;
    for (int x = 0; x < 4; ++x)
        for (int y = 0; y < 4; ++y)
            for (int z = 0; z < 4; ++z)
                for (int w = 0; w < 4; ++w)
                    seen.insert(flat_index(x, y, z, w));
    EXPECT_EQ(static_cast<int>(seen.size()), 256);
}

// ---------- Coord4D ----------------------------------------------------------------

TEST(Coord4DTest, InBounds) {
    EXPECT_TRUE(Coord4D{0,0,0,0}.in_bounds());
    EXPECT_TRUE(Coord4D{3,3,3,3}.in_bounds());
    EXPECT_TRUE(Coord4D{1,2,1,2}.in_bounds());
    EXPECT_FALSE(Coord4D{-1,0,0,0}.in_bounds());
    EXPECT_FALSE(Coord4D{4,0,0,0}.in_bounds());
    EXPECT_FALSE(Coord4D{0,0,0,-1}.in_bounds());
    EXPECT_FALSE(Coord4D{0,0,0,4}.in_bounds());
}

TEST(Coord4DTest, OnBoundary) {
    EXPECT_TRUE(Coord4D{0,1,1,1}.on_boundary());
    EXPECT_TRUE(Coord4D{3,2,1,1}.on_boundary());
    EXPECT_TRUE(Coord4D{1,0,1,1}.on_boundary());
    EXPECT_TRUE(Coord4D{1,3,1,1}.on_boundary());
    EXPECT_TRUE(Coord4D{1,1,0,1}.on_boundary());
    EXPECT_TRUE(Coord4D{1,1,3,1}.on_boundary());
    EXPECT_TRUE(Coord4D{1,1,1,0}.on_boundary());
    EXPECT_TRUE(Coord4D{1,1,1,3}.on_boundary());
    EXPECT_FALSE(Coord4D{1,1,1,1}.on_boundary());
    EXPECT_FALSE(Coord4D{2,2,2,2}.on_boundary());
}

// ---------- Boundary count ---------------------------------------------------------

TEST(BoundaryTest, CountIs240) {
    // |dB| = 4^4 - 2^4 = 256 - 16 = 240
    int boundary = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (coord_4d(i).on_boundary()) ++boundary;
    }
    EXPECT_EQ(boundary, 240);
}

// ---------- Board ------------------------------------------------------------------

TEST(BoardTest, ClearIsEmpty) {
    Board b;
    b.clear();
    for (int i = 0; i < NUM_CELLS; ++i) {
        EXPECT_TRUE(b.at(i).is_none());
    }
}

TEST(BoardTest, SetAndRemovePiece) {
    Board b;
    b.clear();
    Piece p{PieceType::KNIGHT, Color::WHITE};
    b.set_piece(42, p);
    EXPECT_EQ(b.at(42).type, PieceType::KNIGHT);
    EXPECT_EQ(b.at(42).color, Color::WHITE);
    b.remove_piece(42);
    EXPECT_TRUE(b.at(42).is_none());
}

TEST(BoardTest, SetByCoord) {
    Board b;
    b.clear();
    Coord4D c{2, 1, 3, 0};
    b.set_piece(c, {PieceType::ROOK, Color::BLACK});
    EXPECT_EQ(b.at(c).type, PieceType::ROOK);
    EXPECT_EQ(b.at(c).color, Color::BLACK);
    EXPECT_EQ(b.at(flat_index(c)).type, PieceType::ROOK);
}

TEST(BoardTest, FindKing) {
    Board b;
    b.clear();
    b.set_piece(10, {PieceType::KING, Color::WHITE});
    b.set_piece(200, {PieceType::KING, Color::BLACK});
    EXPECT_EQ(b.find_king(Color::WHITE), 10);
    EXPECT_EQ(b.find_king(Color::BLACK), 200);
    EXPECT_EQ(b.find_king(Color::NONE), -1);
}

TEST(BoardTest, InitialPositionPieceCount) {
    Board b;
    b.setup_initial_position();
    int white_count = 0, black_count = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (!b.at(i).is_none()) {
            if (b.at(i).color == Color::WHITE) ++white_count;
            else ++black_count;
        }
    }
    // White: 4 rooks + 4 knights + 4 bishops + 1 queen + 1 king + 16 pawns = 30
    // Black: same = 30
    EXPECT_EQ(white_count, 30);
    EXPECT_EQ(black_count, 30);
}

TEST(BoardTest, InitialPositionKings) {
    Board b;
    b.setup_initial_position();
    int wk = b.find_king(Color::WHITE);
    int bk = b.find_king(Color::BLACK);
    EXPECT_NE(wk, -1);
    EXPECT_NE(bk, -1);
    EXPECT_NE(wk, bk);
}
