#include <gtest/gtest.h>
#include "engine.h"

TEST(EngineTest, NewGame) {
    ChessEngine4D engine;
    engine.new_game();
    EXPECT_FALSE(engine.is_game_over());
    EXPECT_FALSE(engine.is_in_check());
    EXPECT_EQ(engine.side_to_move(), Color::WHITE);
}

TEST(EngineTest, LegalMovesFromStart) {
    ChessEngine4D engine;
    engine.new_game();
    auto moves = engine.get_legal_moves();
    EXPECT_GT(static_cast<int>(moves.size()), 30);
}

TEST(EngineTest, ApplyValidMove) {
    ChessEngine4D engine;
    engine.new_game();
    auto moves = engine.get_legal_moves();
    ASSERT_FALSE(moves.empty());
    bool ok = engine.apply_move(moves[0]);
    EXPECT_TRUE(ok);
    EXPECT_EQ(engine.side_to_move(), Color::BLACK);
}

TEST(EngineTest, RejectInvalidMove) {
    ChessEngine4D engine;
    engine.new_game();
    Move bad{0, 0, 0, PieceType::NONE, PieceType::NONE}; // from==to, no flags
    EXPECT_FALSE(engine.apply_move(bad));
}

TEST(EngineTest, BoardToString) {
    ChessEngine4D engine;
    engine.new_game();
    std::string s = engine.board_to_string();
    EXPECT_FALSE(s.empty());
    // Should contain layer labels
    EXPECT_NE(s.find("w = 0"), std::string::npos);
    EXPECT_NE(s.find("w = 3"), std::string::npos);
}

TEST(EngineTest, ExportImport) {
    ChessEngine4D engine;
    engine.new_game();
    std::string exported = engine.export_state();
    EXPECT_EQ(static_cast<int>(exported.size()), 1 + 256);

    ChessEngine4D engine2;
    engine2.new_game();
    bool ok = engine2.import_state(exported);
    EXPECT_TRUE(ok);
    // Both should have same hash
    EXPECT_EQ(engine.state().hash(), engine2.state().hash());
}

TEST(EngineTest, PerftDepth1) {
    ChessEngine4D engine;
    engine.new_game();
    uint64_t nodes = engine.perft(1);
    EXPECT_EQ(nodes, static_cast<uint64_t>(engine.get_legal_moves().size()));
}

TEST(EngineTest, PerftConsistency) {
    // perft(2) should equal sum of perft(1) for each first move
    ChessEngine4D engine;
    engine.new_game();
    uint64_t p2 = engine.perft(2);

    auto moves = engine.get_legal_moves();
    uint64_t sum = 0;
    for (const auto& m : moves) {
        ChessEngine4D e2;
        e2.new_game();
        e2.apply_move(m);
        sum += e2.perft(1);
    }
    EXPECT_EQ(p2, sum);
}

TEST(EngineTest, MoveToString) {
    Move m{0, 85, QUIET};
    ChessEngine4D engine;
    engine.new_game();
    std::string s = engine.move_to_string(m);
    EXPECT_FALSE(s.empty());
    EXPECT_NE(s.find("->"), std::string::npos);
}
