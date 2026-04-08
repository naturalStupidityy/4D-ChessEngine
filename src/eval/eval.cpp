#include "eval.h"
#include <cmath>

Evaluator::Evaluator() {
    init_pst(pst_pawn_,   10);
    init_pst(pst_knight_, 15);
    init_pst(pst_bishop_, 12);
    init_pst(pst_rook_,    8);
    init_pst(pst_queen_,   6);
    init_pst(pst_king_,    5);
}

// Build a piece-square table: cells closer to the hyper-centre (1.5,1.5,1.5,1.5)
// receive a higher bonus. Max bonus = center_bonus, minimum = 0.
void Evaluator::init_pst(std::array<int, 256>& table, int center_bonus) {
    for (int i = 0; i < NUM_CELLS; ++i) {
        table[i] = centrality_bonus(i) * center_bonus / 3;
    }
}

int Evaluator::centrality_bonus(int flat_idx) const {
    Coord4D c = coord_4d(flat_idx);
    // Chebyshev distance to nearest centre cell: max_i |c_i - 1.5| -> 0..1.5
    // Use integer: max(|2c_i - 3|) ranges from 0 to 3, invert.
    int dist = 0;
    dist = std::max(dist, std::abs(2 * c.x - 3));
    dist = std::max(dist, std::abs(2 * c.y - 3));
    dist = std::max(dist, std::abs(2 * c.z - 3));
    dist = std::max(dist, std::abs(2 * c.w - 3));
    // dist in {0,1,2,3}.  Bonus: 3 - dist gives 3,2,1,0
    return 3 - dist;
}

const std::array<int, 256>* Evaluator::get_pst(PieceType type) const {
    switch (type) {
        case PieceType::PAWN:   return &pst_pawn_;
        case PieceType::KNIGHT: return &pst_knight_;
        case PieceType::BISHOP: return &pst_bishop_;
        case PieceType::ROOK:   return &pst_rook_;
        case PieceType::QUEEN:  return &pst_queen_;
        case PieceType::KING:   return &pst_king_;
        default:                return nullptr;
    }
}

int Evaluator::hyper_centre_control(const Board& board, Color color) const {
    int score = 0;
    // The 16 interior cells: all coords in {1,2}
    for (int x = 1; x <= 2; ++x)
        for (int y = 1; y <= 2; ++y)
            for (int z = 1; z <= 2; ++z)
                for (int w = 1; w <= 2; ++w) {
                    int idx = flat_index(x, y, z, w);
                    const Piece& p = board.at(idx);
                    if (!p.is_none() && p.color == color) {
                        score += 5; // bonus for occupying the hyper-centre
                    }
                }
    return score;
}

int Evaluator::king_safety(const Board& board, Color color) const {
    int king_idx = board.find_king(color);
    if (king_idx < 0) return 0;

    Coord4D kc = coord_4d(king_idx);
    int friends = 0;
    int enemies = 0;

    // Count friendly / enemy pieces in the 80 king-neighbour cells
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            for (int dz = -1; dz <= 1; ++dz)
                for (int dw = -1; dw <= 1; ++dw) {
                    if (dx == 0 && dy == 0 && dz == 0 && dw == 0) continue;
                    int nx = kc.x + dx, ny = kc.y + dy, nz = kc.z + dz, nw = kc.w + dw;
                    if (nx < 0 || nx >= 4 || ny < 0 || ny >= 4 ||
                        nz < 0 || nz >= 4 || nw < 0 || nw >= 4) continue;
                    const Piece& p = board.at(flat_index(nx, ny, nz, nw));
                    if (p.color == color)     ++friends;
                    if (p.color != Color::NONE && p.color != color) ++enemies;
                }

    return friends * 3 - enemies * 6;
}

int Evaluator::evaluate(const Board& board, Color side) const {
    int score = 0;

    for (int i = 0; i < NUM_CELLS; ++i) {
        const Piece& p = board.at(i);
        if (p.is_none()) continue;

        int sign = (p.color == side) ? 1 : -1;

        // Material
        score += sign * MATERIAL_VALUES[static_cast<int>(p.type)];

        // Piece-square table bonus
        const auto* pst = get_pst(p.type);
        if (pst) {
            score += sign * (*pst)[i];
        }
    }

    // Hyper-centre control
    score += hyper_centre_control(board, side);
    score -= hyper_centre_control(board, (side == Color::WHITE) ? Color::BLACK : Color::WHITE);

    // King safety
    score += king_safety(board, side);
    score -= king_safety(board, (side == Color::WHITE) ? Color::BLACK : Color::WHITE);

    return score;
}
