#include <gtest/gtest.h>
#include "types.h"
#include "state.h"
#include "search.h"

TEST(SearchTest, FindBestMoveReturnsValidMove) {
    GameState state;
    state.setup_initial();
    Searcher searcher;
    Move best = searcher.find_best_move(state, 2);
    // The best move should have distinct from/to (or be a cat/phantom event)
    bool valid = (best.from != best.to) ||
                 best.is_cat_spawn() || best.is_phantom_spawn();
    EXPECT_TRUE(valid);
}

TEST(SearchTest, EvaluateNonTrivial) {
    GameState state;
    state.setup_initial();
    Searcher searcher;
    int score = searcher.evaluate_position(state, 1);
    // The score should not be exactly 0 for the starting position
    // (asymmetric piece placement gives a slight imbalance)
    EXPECT_NE(score, 0);
}

TEST(SearchTest, NodesSearchedPositive) {
    GameState state;
    state.setup_initial();
    Searcher searcher;
    searcher.find_best_move(state, 2);
    EXPECT_GT(searcher.nodes_searched(), 0ULL);
}

TEST(SearchTest, TTUsed) {
    GameState state;
    state.setup_initial();
    Searcher searcher;
    searcher.find_best_move(state, 3);
    EXPECT_GT(searcher.tt_size(), 0ULL);
}

TEST(SearchTest, MakeUnmakeConsistent) {
    GameState state;
    state.setup_initial();
    uint64_t hash_before = state.hash();

    Move moves[MAX_MOVES];
    int count = state.generate_legal_moves(moves);
    ASSERT_GT(count, 0);

    // Make and unmake the first legal move
    state.make_move(moves[0]);
    state.unmake_move(moves[0]);

    EXPECT_EQ(state.hash(), hash_before);
    EXPECT_EQ(state.side_to_move(), Color::WHITE);
    EXPECT_EQ(state.fullmove_number(), 1);
}

TEST(SearchTest, ClearTT) {
    Searcher searcher;
    searcher.find_best_move(GameState(), 1);
    EXPECT_GT(searcher.tt_size(), 0ULL);
    searcher.clear_tt();
    EXPECT_EQ(searcher.tt_size(), 0ULL);
}
