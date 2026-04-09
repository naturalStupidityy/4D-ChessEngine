#pragma once
#include <cstdint>
#include <string>
#include <array>

static constexpr int BOARD_SIZE = 4;
static constexpr int NUM_CELLS = 256;  // 4^4
static constexpr int MAX_MOVES = 4096;
static constexpr int MAX_DEPTH = 64;
static constexpr int CAT_PROBABILITY_PERCENT = 8;  // 8% chance per turn

enum class Color : uint8_t { WHITE = 0, BLACK = 1, NONE = 2 };

enum class PieceType : uint8_t {
    NONE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    CAT = 7,
    PHANTOM = 8
};

struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;

    bool operator==(const Piece& o) const { return type == o.type && color == o.color; }
    bool operator!=(const Piece& o) const { return !(*this == o); }
    bool is_none() const { return type == PieceType::NONE; }
    bool has_color() const { return color != Color::NONE; }
    explicit operator bool() const { return type != PieceType::NONE; }
};

struct Coord4D {
    int x, y, z, w;

    bool in_bounds() const {
        return x >= 0 && x < 4 && y >= 0 && y < 4 &&
               z >= 0 && z < 4 && w >= 0 && w < 4;
    }

    bool on_boundary() const {
        return x == 0 || x == 3 || y == 0 || y == 3 ||
               z == 0 || z == 3 || w == 0 || w == 3;
    }

    bool operator==(const Coord4D& o) const {
        return x == o.x && y == o.y && z == o.z && w == o.w;
    }
    bool operator!=(const Coord4D& o) const { return !(*this == o); }
};

// Flat index bijection: phi(x,y,z,w) = x + 4y + 16z + 64w
inline constexpr int flat_index(int x, int y, int z, int w) {
    return x + 4 * y + 16 * z + 64 * w;
}

inline constexpr int flat_index(const Coord4D& c) {
    return c.x + 4 * c.y + 16 * c.z + 64 * c.w;
}

inline constexpr Coord4D coord_4d(int idx) {
    return { idx % 4, (idx / 4) % 4, (idx / 16) % 4, (idx / 64) % 4 };
}

enum MoveFlags : uint8_t {
    QUIET = 0,
    CAPTURE = 1,
    CASTLING = 2,
    EN_PASSANT = 4,
    PROMOTION = 8,
    CAT_SPAWN = 16,
    PHANTOM_SPAWN = 32,
    BOUNDARY_CLEAR = 64
};

struct Move {
    uint8_t from = 0;        // flat index
    uint8_t to = 0;          // flat index
    uint8_t flags = 0;
    PieceType promotion = PieceType::NONE;  // for pawn promotion
    PieceType captured = PieceType::NONE;   // type of captured piece (if any)

    bool is_capture() const { return flags & CAPTURE; }
    bool is_castling() const { return flags & CASTLING; }
    bool is_en_passant() const { return flags & EN_PASSANT; }
    bool is_promotion() const { return flags & PROMOTION; }
    bool is_cat_spawn() const { return flags & CAT_SPAWN; }
    bool is_phantom_spawn() const { return flags & PHANTOM_SPAWN; }

    std::string to_string() const;
};
