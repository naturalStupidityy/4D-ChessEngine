#include "movegen.h"
#include <algorithm>
#include <random>

MoveGenerator::MoveGenerator() {
    king_dirs_initialized_ = false;
}

// --- King direction initialisation -------------------------------------------------
// All Chebyshev distance-1 vectors in 4D: each component in {-1,0,+1}, not all zero.
// 3^4 - 1 = 80 vectors.
void MoveGenerator::init_king_dirs() const {
    int idx = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            for (int dz = -1; dz <= 1; ++dz)
                for (int dw = -1; dw <= 1; ++dw) {
                    if (dx == 0 && dy == 0 && dz == 0 && dw == 0) continue;
                    king_dirs_[idx++] = {dx, dy, dz, dw};
                }
    // self-check: should produce exactly 80
    king_dirs_initialized_ = true;
}

// --- Boundary helper --------------------------------------------------------------
bool MoveGenerator::is_boundary(int idx) {
    Coord4D c = coord_4d(idx);
    return c.on_boundary();
}

// --- Sliding piece generation (Rook / Bishop / Queen) -----------------------------
void MoveGenerator::gen_sliding(const Board& board, Color side, Move* moves, int& count,
                                const Coord4D* dirs, int num_dirs, int from_idx) const {
    Coord4D from = coord_4d(from_idx);
    for (int d = 0; d < num_dirs; ++d) {
        Coord4D cur = {from.x + dirs[d].x, from.y + dirs[d].y,
                       from.z + dirs[d].z, from.w + dirs[d].w};
        while (cur.in_bounds()) {
            int to_idx = flat_index(cur);
            const Piece& target = board.at(to_idx);
            if (target.is_none()) {
                moves[count++] = {static_cast<uint8_t>(from_idx),
                                  static_cast<uint8_t>(to_idx), QUIET};
            } else {
                if (target.color != side) {
                    moves[count++] = {static_cast<uint8_t>(from_idx),
                                      static_cast<uint8_t>(to_idx),
                                      CAPTURE, PieceType::NONE, target.type};
                }
                break; // blocked
            }
            cur.x += dirs[d].x;
            cur.y += dirs[d].y;
            cur.z += dirs[d].z;
            cur.w += dirs[d].w;
        }
    }
}

// --- Stepping piece generation (single-step along given dirs) ---------------------
void MoveGenerator::gen_stepping(const Board& board, Color side, Move* moves, int& count,
                                 const Coord4D* dirs, int num_dirs, int from_idx) const {
    Coord4D from = coord_4d(from_idx);
    for (int d = 0; d < num_dirs; ++d) {
        Coord4D to = {from.x + dirs[d].x, from.y + dirs[d].y,
                      from.z + dirs[d].z, from.w + dirs[d].w};
        if (!to.in_bounds()) continue;
        int to_idx = flat_index(to);
        const Piece& target = board.at(to_idx);
        if (target.is_none()) {
            moves[count++] = {static_cast<uint8_t>(from_idx),
                              static_cast<uint8_t>(to_idx), QUIET};
        } else if (target.color != side) {
            moves[count++] = {static_cast<uint8_t>(from_idx),
                              static_cast<uint8_t>(to_idx),
                              CAPTURE, PieceType::NONE, target.type};
        }
    }
}

// --- Knight (leaping) --------------------------------------------------------------
void MoveGenerator::gen_knight(const Board& board, Color side, Move* moves, int& count,
                               int from_idx) const {
    Coord4D from = coord_4d(from_idx);
    for (int d = 0; d < 48; ++d) {
        Coord4D to = {from.x + KNIGHT_DIRS[d].x, from.y + KNIGHT_DIRS[d].y,
                      from.z + KNIGHT_DIRS[d].z, from.w + KNIGHT_DIRS[d].w};
        if (!to.in_bounds()) continue;
        int to_idx = flat_index(to);
        const Piece& target = board.at(to_idx);
        if (target.is_none()) {
            moves[count++] = {static_cast<uint8_t>(from_idx),
                              static_cast<uint8_t>(to_idx), QUIET};
        } else if (target.color != side) {
            moves[count++] = {static_cast<uint8_t>(from_idx),
                              static_cast<uint8_t>(to_idx),
                              CAPTURE, PieceType::NONE, target.type};
        }
    }
}

// --- King (single step in 80 directions) ------------------------------------------
void MoveGenerator::gen_king(const Board& board, Color side, Move* moves, int& count,
                             int from_idx) const {
    if (!king_dirs_initialized_) {
        // const_cast is safe because we only init once; the data is logically const
        const_cast<MoveGenerator*>(this)->init_king_dirs();
    }
    gen_stepping(board, side, moves, count, king_dirs_, NUM_KING_DIRS, from_idx);
}

// --- Pawn -------------------------------------------------------------------------
// White pawns move in +w direction (towards w=3), promote on w=3.
// Black pawns move in -w direction (towards w=0), promote on w=0.
// Forward: (0,0,0,+1) for white, (0,0,0,-1) for black.
// Double push from start: white from w=1, black from w=2.
// Captures: change w by +-1 AND change one of {x,y,z} by +-1  (6 capture dirs each).
void MoveGenerator::gen_pawn(const Board& board, Color side, Move* moves, int& count,
                             int from_idx) const {
    Coord4D from = coord_4d(from_idx);
    int w_dir   = (side == Color::WHITE) ?  1 : -1;
    int start_w = (side == Color::WHITE) ?  1 :  2;
    int promo_w = (side == Color::WHITE) ?  3 :  0;

    // --- Single push ---
    int to_w = from.w + w_dir;
    if (to_w >= 0 && to_w < 4) {
        int to_idx = flat_index(from.x, from.y, from.z, to_w);
        if (board.at(to_idx).is_none()) {
            if (to_w == promo_w) {
                // Generate 4 promotion moves
                PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK,
                                      PieceType::BISHOP, PieceType::KNIGHT};
                for (auto pt : promos) {
                    moves[count++] = {static_cast<uint8_t>(from_idx),
                                      static_cast<uint8_t>(to_idx),
                                      PROMOTION, pt, PieceType::NONE};
                }
            } else {
                moves[count++] = {static_cast<uint8_t>(from_idx),
                                  static_cast<uint8_t>(to_idx), QUIET};

                // --- Double push from starting rank ---
                if (from.w == start_w) {
                    int to_w2 = from.w + 2 * w_dir;
                    if (to_w2 >= 0 && to_w2 < 4) {
                        int to_idx2 = flat_index(from.x, from.y, from.z, to_w2);
                        if (board.at(to_idx2).is_none()) {
                            moves[count++] = {static_cast<uint8_t>(from_idx),
                                              static_cast<uint8_t>(to_idx2), QUIET};
                        }
                    }
                }
            }
        }
    }

    // --- Captures: w +-1 combined with one of {x,y,z} +-1 ---
    // That gives 2 (w sign) * 3 (axis choice) * 2 (sign) = 12 capture directions,
    // but w_dir is fixed per side so 3*2 = 6 capture directions.
    int signs[2] = {-1, 1};

    for (int a = 0; a < 3; ++a) {
        for (int s = 0; s < 2; ++s) {
            int coords[4] = {from.x, from.y, from.z, from.w + w_dir};
            if      (a == 0) coords[0] += signs[s];
            else if (a == 1) coords[1] += signs[s];
            else              coords[2] += signs[s];

            Coord4D to = {coords[0], coords[1], coords[2], coords[3]};
            if (!to.in_bounds()) continue;

            int to_idx = flat_index(to);
            const Piece& target = board.at(to_idx);

            if (!target.is_none() && target.color != side && target.type != PieceType::KING) {
                if (to.w == promo_w) {
                    PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK,
                                          PieceType::BISHOP, PieceType::KNIGHT};
                    for (auto pt : promos) {
                        moves[count++] = {static_cast<uint8_t>(from_idx),
                                          static_cast<uint8_t>(to_idx),
                                          PROMOTION, pt, target.type};
                    }
                } else {
                    moves[count++] = {static_cast<uint8_t>(from_idx),
                                      static_cast<uint8_t>(to_idx),
                                      CAPTURE, PieceType::NONE, target.type};
                }
            }
        }
    }
}

// --- Phantom (moves to any empty interior cell) -----------------------------------
void MoveGenerator::gen_phantom(const Board& board, Color /*side*/, Move* moves, int& count,
                                int from_idx) const {
    // Phantom can relocate to any empty non-boundary cell.
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (i == from_idx) continue;
        if (is_boundary(i)) continue; // phantom only on interior
        if (!board.at(i).is_none()) continue;
        moves[count++] = {static_cast<uint8_t>(from_idx),
                          static_cast<uint8_t>(i), QUIET};
    }
}

// --- is_square_attacked ----------------------------------------------------------
bool MoveGenerator::is_square_attacked(const Board& board, int idx, Color attacker) const {
    Coord4D sq = coord_4d(idx);

    // Check rook-like rays (8 axis-aligned)
    for (int d = 0; d < 8; ++d) {
        Coord4D cur = {sq.x + ROOK_DIRS[d].x, sq.y + ROOK_DIRS[d].y,
                       sq.z + ROOK_DIRS[d].z, sq.w + ROOK_DIRS[d].w};
        while (cur.in_bounds()) {
            int ci = flat_index(cur);
            const Piece& p = board.at(ci);
            if (!p.is_none()) {
                if (p.color == attacker &&
                    (p.type == PieceType::ROOK || p.type == PieceType::QUEEN))
                    return true;
                break;
            }
            cur.x += ROOK_DIRS[d].x; cur.y += ROOK_DIRS[d].y;
            cur.z += ROOK_DIRS[d].z; cur.w += ROOK_DIRS[d].w;
        }
    }

    // Check bishop-like rays (24 face-diagonals)
    for (int d = 0; d < 24; ++d) {
        Coord4D cur = {sq.x + BISHOP_DIRS[d].x, sq.y + BISHOP_DIRS[d].y,
                       sq.z + BISHOP_DIRS[d].z, sq.w + BISHOP_DIRS[d].w};
        while (cur.in_bounds()) {
            int ci = flat_index(cur);
            const Piece& p = board.at(ci);
            if (!p.is_none()) {
                if (p.color == attacker &&
                    (p.type == PieceType::BISHOP || p.type == PieceType::QUEEN))
                    return true;
                break;
            }
            cur.x += BISHOP_DIRS[d].x; cur.y += BISHOP_DIRS[d].y;
            cur.z += BISHOP_DIRS[d].z; cur.w += BISHOP_DIRS[d].w;
        }
    }

    // Check knight attacks (48 vectors)
    for (int d = 0; d < 48; ++d) {
        Coord4D to = {sq.x + KNIGHT_DIRS[d].x, sq.y + KNIGHT_DIRS[d].y,
                      sq.z + KNIGHT_DIRS[d].z, sq.w + KNIGHT_DIRS[d].w};
        if (to.in_bounds()) {
            const Piece& p = board.at(flat_index(to));
            if (p.color == attacker && p.type == PieceType::KNIGHT)
                return true;
        }
    }

    // Check king attacks (80 directions)
    if (!king_dirs_initialized_) {
        const_cast<MoveGenerator*>(this)->init_king_dirs();
    }
    for (int d = 0; d < NUM_KING_DIRS; ++d) {
        Coord4D to = {sq.x + king_dirs_[d].x, sq.y + king_dirs_[d].y,
                      sq.z + king_dirs_[d].z, sq.w + king_dirs_[d].w};
        if (to.in_bounds()) {
            const Piece& p = board.at(flat_index(to));
            if (p.color == attacker && p.type == PieceType::KING)
                return true;
        }
    }

    // Check pawn attacks
    // A pawn of 'attacker' colour attacks squares that are one w-step in its advance
    // direction and one step in x, y, or z.
    int pw = (attacker == Color::WHITE) ? -1 : 1; // the square's w relative to the pawn
    int signs[2] = {-1, 1};
    for (int a = 0; a < 3; ++a) {
        for (int s = 0; s < 2; ++s) {
            int c[4] = {sq.x, sq.y, sq.z, sq.w + pw};
            if      (a == 0) c[0] += signs[s];
            else if (a == 1) c[1] += signs[s];
            else              c[2] += signs[s];
            if (c[0] >= 0 && c[0] < 4 && c[1] >= 0 && c[1] < 4 &&
                c[2] >= 0 && c[2] < 4 && c[3] >= 0 && c[3] < 4) {
                const Piece& p = board.at(flat_index(c[0], c[1], c[2], c[3]));
                if (p.color == attacker && p.type == PieceType::PAWN)
                    return true;
            }
        }
    }

    return false;
}

// --- is_in_check -----------------------------------------------------------------
bool MoveGenerator::is_in_check(const Board& board, Color color) const {
    Color opp = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    int king_idx = board.find_king(color);
    if (king_idx < 0) return false; // no king on board
    return is_square_attacked(board, king_idx, opp);
}

// --- generate_pseudo_legal --------------------------------------------------------
int MoveGenerator::generate_pseudo_legal(const Board& board, Color side, Move* moves) const {
    int count = 0;
    for (int i = 0; i < NUM_CELLS; ++i) {
        const Piece& p = board.at(i);
        if (p.is_none() || p.color != side) continue;

        switch (p.type) {
            case PieceType::ROOK:
                gen_sliding(board, side, moves, count, ROOK_DIRS, 8, i);
                break;
            case PieceType::BISHOP:
                gen_sliding(board, side, moves, count, BISHOP_DIRS, 24, i);
                break;
            case PieceType::QUEEN:
                gen_sliding(board, side, moves, count, QUEEN_DIRS, 32, i);
                break;
            case PieceType::KNIGHT:
                gen_knight(board, side, moves, count, i);
                break;
            case PieceType::KING:
                gen_king(board, side, moves, count, i);
                break;
            case PieceType::PAWN:
                gen_pawn(board, side, moves, count, i);
                break;
            case PieceType::PHANTOM:
                gen_phantom(board, side, moves, count, i);
                break;
            case PieceType::CAT:
                // Cat is a volatile, stationary piece — no moves generated
                break;
            default:
                break;
        }
    }
    return count;
}

// --- is_legal_move ----------------------------------------------------------------
bool MoveGenerator::is_legal_move(const Board& board, Color side, const Move& move) const {
    // Cat spawn / phantom spawn are always "legal" events
    if (move.is_cat_spawn() || move.is_phantom_spawn()) return true;

    // Make the move on a copy and check if own king is still safe
    Board copy = board;

    // Move the piece
    Piece from_piece = copy.at(move.from);
    copy.at(move.to) = from_piece;

    // Handle promotion
    if (move.is_promotion()) {
        copy.at(move.to).type = move.promotion;
    }

    // Remove from origin (unless en-passant which targets a different square)
    copy.remove_piece(move.from);

    // Handle en-passant capture
    if (move.is_en_passant()) {
        // The captured pawn is on the same w-layer as from, at the to x,y,z
        Coord4D to = coord_4d(move.to);
        Coord4D ep_pawn = {to.x, to.y, to.z, coord_4d(move.from).w};
        copy.remove_piece(flat_index(ep_pawn));
    }

    return !is_in_check(copy, side);
}

// --- generate_legal ---------------------------------------------------------------
int MoveGenerator::generate_legal(const Board& board, Color side, Move* moves) const {
    Move pseudo[MAX_MOVES];
    int pseudo_count = generate_pseudo_legal(board, side, pseudo);

    int legal_count = 0;
    for (int i = 0; i < pseudo_count; ++i) {
        if (is_legal_move(board, side, pseudo[i])) {
            moves[legal_count++] = pseudo[i];
        }
    }
    return legal_count;
}

// --- perft ------------------------------------------------------------------------
uint64_t MoveGenerator::perft(const Board& board, Color side, int depth) const {
    if (depth == 0) return 1;

    Move moves[MAX_MOVES];
    int count = generate_legal(board, side, moves);

    if (depth == 1) return static_cast<uint64_t>(count);

    uint64_t nodes = 0;
    Color opp = (side == Color::WHITE) ? Color::BLACK : Color::WHITE;
    for (int i = 0; i < count; ++i) {
        Board copy = board;

        // Apply move on copy
        Piece from_piece = copy.at(moves[i].from);
        copy.at(moves[i].to) = from_piece;
        if (moves[i].is_promotion()) {
            copy.at(moves[i].to).type = moves[i].promotion;
        }
        copy.remove_piece(moves[i].from);

        // En-passant
        if (moves[i].is_en_passant()) {
            Coord4D to = coord_4d(moves[i].to);
            Coord4D ep_pawn = {to.x, to.y, to.z, coord_4d(moves[i].from).w};
            copy.remove_piece(flat_index(ep_pawn));
        }

        nodes += perft(copy, opp, depth - 1);
    }
    return nodes;
}

// --- Cat spawn chance -------------------------------------------------------------
bool MoveGenerator::should_spawn_cat() const {
    static thread_local std::mt19937 rng(42);
    return (rng() % 100) < CAT_PROBABILITY_PERCENT;
}

// --- Boundary enemy cells for cat -------------------------------------------------
std::vector<int> MoveGenerator::get_boundary_enemy_cells(const Board& board, Color cat_color) const {
    std::vector<int> result;
    Color enemy = (cat_color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (!is_boundary(i)) continue;
        const Piece& p = board.at(i);
        if (!p.is_none() && p.color == enemy) {
            result.push_back(i);
        }
    }
    return result;
}
