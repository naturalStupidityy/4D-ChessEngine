#include "engine.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

ChessEngine4D::ChessEngine4D()
    : rng_(std::random_device{}()) {}

void ChessEngine4D::new_game() {
    state_.setup_initial();
    searcher_.clear_tt();
}

std::vector<Move> ChessEngine4D::get_legal_moves() const {
    std::vector<Move> result;
    Move moves[MAX_MOVES];
    int count = state_.generate_legal_moves(moves);
    result.assign(moves, moves + count);
    return result;
}

bool ChessEngine4D::apply_move(const Move& move) {
    // Validate the move is legal
    Move legal[MAX_MOVES];
    int count = state_.generate_legal_moves(legal);
    bool found = false;
    for (int i = 0; i < count; ++i) {
        if (legal[i].from == move.from && legal[i].to == move.to &&
            legal[i].flags == move.flags) {
            found = true;
            break;
        }
    }
    if (!found) return false;

    state_.make_move(move);
    return true;
}

Move ChessEngine4D::get_best_move(int depth) {
    return searcher_.find_best_move(state_, depth);
}

bool ChessEngine4D::process_special_events() {
    bool any = false;
    if (state_.trigger_cat_event(rng_))     any = true;
    if (state_.trigger_phantom_event(rng_))  any = true;
    return any;
}

bool ChessEngine4D::is_game_over() const {
    return state_.is_game_over();
}

bool ChessEngine4D::is_in_check() const {
    return state_.in_check();
}

Color ChessEngine4D::side_to_move() const {
    return state_.side_to_move();
}

std::string ChessEngine4D::move_to_string(const Move& move) const {
    return move.to_string();
}

std::string ChessEngine4D::board_to_string() const {
    std::ostringstream ss;
    const char* piece_char = " pnbrqkC?X?"; // none, pawn, knight, bishop, rook, queen, king, cat, phantom
    // White = uppercase, Black = lowercase

    ss << "=== 4D Chess Board [4]^4 ===\n\n";

    for (int w = 0; w < 4; ++w) {
        ss << "--- w = " << w << " layer ---\n";
        for (int z = 0; z < 4; ++z) {
            ss << "  z=" << z << " :";
            for (int y = 0; y < 4; ++y) {
                ss << "  ";
                for (int x = 0; x < 4; ++x) {
                    const Piece& p = state_.board().at(flat_index(x, y, z, w));
                    if (p.is_none()) {
                        ss << " . ";
                    } else {
                        char c = piece_char[static_cast<int>(p.type)];
                        if (p.color == Color::WHITE) c = static_cast<char>(std::toupper(c));
                        else                         c = static_cast<char>(std::tolower(c));
                        ss << ' ' << c << ' ';
                    }
                }
                if (y < 3) ss << '|';
            }
            ss << "\n";
        }
        ss << "\n";
    }

    ss << "Side to move: "
       << (state_.side_to_move() == Color::WHITE ? "WHITE" : "BLACK") << "\n";
    ss << "Hash: 0x" << std::hex << std::setfill('0') << std::setw(16)
       << state_.hash() << std::dec << "\n";

    return ss.str();
}

std::string ChessEngine4D::export_state() const {
    // Compact serialization: for each of 256 cells, write type|color as one byte
    // type (4 bits) | color (2 bits) | unused (2 bits)
    std::string result;
    result.reserve(258);
    result += static_cast<char>((state_.side_to_move() == Color::WHITE) ? 0 : 1);

    for (int i = 0; i < NUM_CELLS; ++i) {
        const Piece& p = state_.board().at(i);
        uint8_t byte = (static_cast<uint8_t>(p.type) << 4) |
                       (static_cast<uint8_t>(p.color) << 0);
        result += static_cast<char>(byte);
    }
    return result;
}

bool ChessEngine4D::import_state(const std::string& state_str) {
    if (static_cast<int>(state_str.size()) != 1 + NUM_CELLS) return false;

    state_.board().clear();
    state_.set_side_to_move((state_str[0] == 0) ? Color::WHITE : Color::BLACK);

    for (int i = 0; i < NUM_CELLS; ++i) {
        uint8_t byte = static_cast<uint8_t>(state_str[i + 1]);
        PieceType type = static_cast<PieceType>(byte >> 4);
        Color color   = static_cast<Color>(byte & 0x03);
        if (type != PieceType::NONE) {
            state_.board().at(i) = {type, color};
        }
    }

    state_.recompute_hash();
    return true;
}

uint64_t ChessEngine4D::perft(int depth) const {
    return state_.movegen().perft(state_.board(), state_.side_to_move(), depth);
}
