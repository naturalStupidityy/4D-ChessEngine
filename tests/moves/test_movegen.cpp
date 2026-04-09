#include <gtest/gtest.h>
#include "types.h"
#include "board.h"
#include "movegen.h"

// ---------- Knight vector count ----------------------------------------------------

TEST(MovegenTest, KnightVectorCount) {
    // The spec says exactly 48 distinct knight vectors
    EXPECT_EQ(static_cast<int>(sizeof(MoveGenerator::KNIGHT_DIRS) / sizeof(Coord4D)), 48);
}

// ---------- King direction count ---------------------------------------------------

TEST(MovegenTest, KingDirectionCount) {
    MoveGenerator mg;
    // Force initialisation of king dirs by generating a move for a king on any board
    Board b;
    b.clear();
    b.set_piece(0, {PieceType::KING, Color::WHITE});
    Move moves[256];
    int count = mg.generate_pseudo_legal(b, Color::WHITE, moves);
    // From a corner (0,0,0,0), the king can step to 3^4 - 1 = 80 neighbours,
    // but some are out of bounds. From (0,0,0,0) only half are valid: 2^4=16 in bounds.
    // We just check that directions were initialised and moves were generated.
    EXPECT_GT(count, 0);
}

// ---------- Rook move count --------------------------------------------------------

TEST(MovegenTest, RookFromCenter) {
    Board b;
    b.clear();
    // Place a rook at the exact center: (1,1,1,1) = flat_index 1+4+16+64 = 85
    int center = flat_index(1, 1, 1, 1);
    b.set_piece(center, {PieceType::ROOK, Color::WHITE});
    MoveGenerator mg;
    Move moves[256];
    int count = mg.generate_pseudo_legal(b, Color::WHITE, moves);
    // From center, rook can go 3 cells in each of 8 axis directions = 24 moves
    EXPECT_EQ(count, 24);
}

// ---------- Bishop move count ------------------------------------------------------

TEST(MovegenTest, BishopFromCenter) {
    Board b;
    b.clear();
    int center = flat_index(1, 1, 1, 1);
    b.set_piece(center, {PieceType::BISHOP, Color::WHITE});
    MoveGenerator mg;
    Move moves[512];
    int count = mg.generate_pseudo_legal(b, Color::WHITE, moves);
    // From center, bishop can go up to 1 cell in each of 24 face-diagonal dirs = 24
    EXPECT_EQ(count, 24);
}

// ---------- Queen move count -------------------------------------------------------

TEST(MovegenTest, QueenFromCenter) {
    Board b;
    b.clear();
    int center = flat_index(1, 1, 1, 1);
    b.set_piece(center, {PieceType::QUEEN, Color::WHITE});
    MoveGenerator mg;
    Move moves[512];
    int count = mg.generate_pseudo_legal(b, Color::WHITE, moves);
    // Queen = rook(24) + bishop(24) = 48
    EXPECT_EQ(count, 48);
}

// ---------- Boundary helper --------------------------------------------------------

TEST(MovegenTest, BoundaryHelper) {
    // Spot-check a few indices
    EXPECT_TRUE(MoveGenerator::is_boundary(0));      // (0,0,0,0) — all coords on boundary
    EXPECT_TRUE(MoveGenerator::is_boundary(255));    // (3,3,3,3) — all coords on boundary
    EXPECT_FALSE(MoveGenerator::is_boundary(85));    // (1,1,1,1) — interior
    EXPECT_FALSE(MoveGenerator::is_boundary(flat_index(2, 2, 2, 2))); // interior
}

// ---------- is_square_attacked -----------------------------------------------------

TEST(MovegenTest, IsSquareAttacked) {
    Board b;
    b.clear();
    MoveGenerator mg;

    // Rook attacks along axis
    b.set_piece(0, {PieceType::ROOK, Color::WHITE});     // (0,0,0,0)
    b.set_piece(flat_index(2,0,0,0), {PieceType::QUEEN, Color::BLACK}); // (2,0,0,0)
    EXPECT_TRUE(mg.is_square_attacked(b, flat_index(2,0,0,0), Color::WHITE));

    // Knight attacks
    Board b2; b2.clear();
    b2.set_piece(0, {PieceType::KNIGHT, Color::WHITE});  // (0,0,0,0)
    int knight_target = flat_index(1, 2, 0, 0);          // (1,2,0,0)
    EXPECT_TRUE(mg.is_square_attacked(b2, knight_target, Color::WHITE));

    // Pawn attacks
    Board b3; b3.clear();
    b3.set_piece(flat_index(1, 0, 0, 1), {PieceType::PAWN, Color::WHITE}); // (1,0,0,1)
    // White pawn at w=1 attacks cells at w=2 with one of x,y,z changed by +/-1
    int pawn_target = flat_index(2, 0, 0, 2); // (2,0,0,2)
    EXPECT_TRUE(mg.is_square_attacked(b3, pawn_target, Color::WHITE));
}

// ---------- Initial position legal moves -------------------------------------------

TEST(MovegenTest, LegalMovesFromStart) {
    Board b;
    b.setup_initial_position();
    MoveGenerator mg;
    Move moves[MAX_MOVES];
    int count = mg.generate_legal(b, Color::WHITE, moves);
    // White has 16 pawns, each can push 1 or 2 = 32 pawn moves
    // + rook/knight/bishop/queen/king moves
    EXPECT_GT(count, 30);
}

// ---------- Pawn promotion ---------------------------------------------------------

TEST(MovegenTest, PawnPromotion) {
    Board b;
    b.clear();
    // White pawn on w=2 (one step from promotion w=3)
    b.set_piece(flat_index(0, 0, 0, 2), {PieceType::PAWN, Color::WHITE});
    b.set_piece(200, {PieceType::KING, Color::WHITE});
    b.set_piece(210, {PieceType::KING, Color::BLACK});
    MoveGenerator mg;
    Move moves[MAX_MOVES];
    int count = mg.generate_pseudo_legal(b, Color::WHITE, moves);
    int promos = 0;
    for (int i = 0; i < count; ++i) {
        if (moves[i].is_promotion()) ++promos;
    }
    // Single push to w=3 gives 4 promotions (Q,R,B,N)
    EXPECT_EQ(promos, 4);
}
