#include "engine.h"
#include <iostream>
#include <chrono>

static void print_usage(const char* prog) {
    std::cout << "4D Chess Engine — Hypercubic Board [4]^4\n";
    std::cout << "Usage:\n";
    std::cout << "  " << prog << "              Run perft + AI demo\n";
    std::cout << "  " << prog << " perft <d>     Run perft to depth d\n";
    std::cout << "  " << prog << " ai <depth>    Run AI vs AI for a few moves\n";
    std::cout << "  " << prog << " board         Print the starting board\n";
    std::cout << "  " << prog << " test          Run unit tests\n";
}

int main(int argc, char* argv[]) {
    ChessEngine4D engine;
    engine.new_game();

    if (argc >= 2) {
        std::string cmd = argv[1];

        if (cmd == "board") {
            std::cout << engine.board_to_string();
            return 0;
        }

        if (cmd == "perft") {
            int depth = (argc >= 3) ? std::atoi(argv[2]) : 3;
            for (int d = 1; d <= depth; ++d) {
                auto t0 = std::chrono::steady_clock::now();
                uint64_t nodes = engine.perft(d);
                auto t1 = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
                std::cout << "perft(" << d << ") = " << nodes
                          << "  (" << ms << " ms)" << std::endl;
            }
            return 0;
        }

        if (cmd == "ai") {
            int depth = (argc >= 3) ? std::atoi(argv[2]) : 4;
            std::cout << engine.board_to_string();

            for (int turn = 0; turn < 10; ++turn) {
                if (engine.is_game_over()) {
                    std::cout << "Game over!\n";
                    break;
                }

                auto t0 = std::chrono::steady_clock::now();
                Move best = engine.get_best_move(depth);
                auto t1 = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

                if (best.from == best.to && !best.is_cat_spawn() && !best.is_phantom_spawn()) {
                    std::cout << "No legal moves.\n";
                    break;
                }

                std::cout << (engine.side_to_move() == Color::WHITE ? "White" : "Black")
                          << " plays: " << engine.move_to_string(best)
                          << "  (" << ms << " ms, "
                          << "TT size: " << engine.state().hash() << ")\n";

                engine.apply_move(best);

                if (engine.process_special_events()) {
                    std::cout << "  ** Special event triggered! **\n";
                }
            }

            std::cout << "\nFinal position:\n";
            std::cout << engine.board_to_string();
            return 0;
        }

        print_usage(argv[0]);
        return 1;
    }

    // --- Default: quick demo ----------------------------------------------------------
    std::cout << "============================================\n";
    std::cout << "  4D Chess Engine — Hypercubic Board [4]^4\n";
    std::cout << "============================================\n\n";

    std::cout << engine.board_to_string();

    auto moves = engine.get_legal_moves();
    std::cout << "Legal moves from starting position: " << moves.size() << "\n\n";

    // Perft test
    std::cout << "--- Perft Results ---\n";
    for (int d = 1; d <= 3; ++d) {
        auto t0 = std::chrono::steady_clock::now();
        uint64_t nodes = engine.perft(d);
        auto t1 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << "  perft(" << d << ") = " << nodes
                  << "  (" << ms << " ms)" << std::endl;
    }

    // AI vs AI (2 moves each)
    std::cout << "\n--- AI vs AI (depth=3, 4 half-moves) ---\n";
    for (int i = 0; i < 4; ++i) {
        if (engine.is_game_over()) { std::cout << "Game over!\n"; break; }

        auto t0 = std::chrono::steady_clock::now();
        Move best = engine.get_best_move(3);
        auto t1 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        if (best.from == best.to && !best.is_cat_spawn() && !best.is_phantom_spawn()) break;

        std::cout << "  Move " << (i + 1) << " ("
                  << (engine.side_to_move() == Color::WHITE ? "W" : "B") << "): "
                  << engine.move_to_string(best) << "  [" << ms << " ms]\n";

        engine.apply_move(best);
        engine.process_special_events();
    }

    std::cout << "\n" << engine.board_to_string();
    return 0;
}
