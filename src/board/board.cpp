#include "board.h"

Board::Board() {
    clear();
}

void Board::clear() {
    for (auto& cell : cells_) {
        cell = Piece{};
    }
}

void Board::set_piece(int flat_idx, const Piece& piece) {
    if (valid_index(flat_idx)) {
        cells_[flat_idx] = piece;
    }
}

void Board::set_piece(const Coord4D& c, const Piece& piece) {
    if (c.in_bounds()) {
        cells_[flat_index(c)] = piece;
    }
}

void Board::remove_piece(int flat_idx) {
    if (valid_index(flat_idx)) {
        cells_[flat_idx] = Piece{};
    }
}

void Board::remove_piece(const Coord4D& c) {
    if (c.in_bounds()) {
        cells_[flat_index(c)] = Piece{};
    }
}

int Board::find_king(Color color) const {
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (cells_[i].type == PieceType::KING && cells_[i].color == color) {
            return i;
        }
    }
    return -1;
}

bool Board::is_boundary(int flat_idx) const {
    if (!valid_index(flat_idx)) return false;
    Coord4D c = coord_4d(flat_idx);
    return c.on_boundary();
}

void Board::setup_initial_position() {
    clear();

    // === WHITE PIECES (w=0 and w=1 layers) ===

    // w=0 layer: back rank pieces
    // Rooks at corners of w=0
    set_piece({0, 0, 0, 0}, {PieceType::ROOK, Color::WHITE});
    set_piece({3, 0, 0, 0}, {PieceType::ROOK, Color::WHITE});
    set_piece({0, 3, 0, 0}, {PieceType::ROOK, Color::WHITE});
    set_piece({3, 3, 0, 0}, {PieceType::ROOK, Color::WHITE});

    // Knights next to rooks
    set_piece({1, 0, 0, 0}, {PieceType::KNIGHT, Color::WHITE});
    set_piece({2, 0, 0, 0}, {PieceType::KNIGHT, Color::WHITE});
    set_piece({0, 1, 0, 0}, {PieceType::KNIGHT, Color::WHITE});
    set_piece({0, 2, 0, 0}, {PieceType::KNIGHT, Color::WHITE});

    // Bishops
    set_piece({1, 1, 0, 0}, {PieceType::BISHOP, Color::WHITE});
    set_piece({2, 2, 0, 0}, {PieceType::BISHOP, Color::WHITE});
    set_piece({1, 2, 0, 0}, {PieceType::BISHOP, Color::WHITE});
    set_piece({2, 1, 0, 0}, {PieceType::BISHOP, Color::WHITE});

    // Queen and King in center of w=0
    set_piece({2, 1, 1, 0}, {PieceType::QUEEN, Color::WHITE});
    set_piece({1, 2, 1, 0}, {PieceType::KING, Color::WHITE});

    // w=1 layer: all 16 cells are pawns
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            for (int z = 0; z < 4; z++) {
                set_piece({x, y, z, 1}, {PieceType::PAWN, Color::WHITE});
            }
        }
    }

    // === BLACK PIECES (w=3 and w=2 layers) ===

    // w=3 layer: back rank pieces (mirrored from white)
    set_piece({0, 0, 0, 3}, {PieceType::ROOK, Color::BLACK});
    set_piece({3, 0, 0, 3}, {PieceType::ROOK, Color::BLACK});
    set_piece({0, 3, 0, 3}, {PieceType::ROOK, Color::BLACK});
    set_piece({3, 3, 0, 3}, {PieceType::ROOK, Color::BLACK});

    set_piece({1, 0, 0, 3}, {PieceType::KNIGHT, Color::BLACK});
    set_piece({2, 0, 0, 3}, {PieceType::KNIGHT, Color::BLACK});
    set_piece({0, 1, 0, 3}, {PieceType::KNIGHT, Color::BLACK});
    set_piece({0, 2, 0, 3}, {PieceType::KNIGHT, Color::BLACK});

    set_piece({1, 1, 0, 3}, {PieceType::BISHOP, Color::BLACK});
    set_piece({2, 2, 0, 3}, {PieceType::BISHOP, Color::BLACK});
    set_piece({1, 2, 0, 3}, {PieceType::BISHOP, Color::BLACK});
    set_piece({2, 1, 0, 3}, {PieceType::BISHOP, Color::BLACK});

    set_piece({1, 2, 1, 3}, {PieceType::QUEEN, Color::BLACK});
    set_piece({2, 1, 1, 3}, {PieceType::KING, Color::BLACK});

    // w=2 layer: all 16 cells are pawns
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            for (int z = 0; z < 4; z++) {
                set_piece({x, y, z, 2}, {PieceType::PAWN, Color::BLACK});
            }
        }
    }
}
