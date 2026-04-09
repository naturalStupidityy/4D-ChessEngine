#include "state.h"
#include <cstring>
#include <algorithm>

GameState::GameState()
    : side_to_move_(Color::WHITE),
      current_hash_(0),
      halfmove_clock_(0),
      fullmove_number_(1),
      castling_rights_(0xFF),
      en_passant_target_(255) {}

void GameState::setup_initial() {
    board_.setup_initial_position();
    side_to_move_ = Color::WHITE;
    halfmove_clock_ = 0;
    fullmove_number_ = 1;
    castling_rights_ = 0xFF;
    en_passant_target_ = 255;
    compute_hash();
}

void GameState::compute_hash() {
    current_hash_ = zobrist_.compute_hash(board_, side_to_move_);
}

// --- make_move --------------------------------------------------------------------
void GameState::make_move(const Move& move) {
    UndoInfo undo;
    undo.from = move.from;
    undo.to = move.to;
    undo.flags = static_cast<MoveFlags>(move.flags);
    undo.hash = current_hash_;
    undo.promotion = move.promotion;
    undo.halfmove_clock = halfmove_clock_;
    undo.castling_rights = castling_rights_;
    undo.en_passant_target = en_passant_target_;

    Piece from_piece = board_.at(move.from);
    Piece to_piece = board_.at(move.to);
    undo.captured = to_piece;

    // XOR out the piece that is moving from its origin
    current_hash_ ^= zobrist_.piece_hash(from_piece.type, from_piece.color, move.from);

    // If there is a captured piece, XOR it out and mark halfmove reset
    if (!to_piece.is_none()) {
        current_hash_ ^= zobrist_.piece_hash(to_piece.type, to_piece.color, move.to);
        halfmove_clock_ = 0;
    }

    // Move the piece
    PieceType moving_type = from_piece.type;
    board_.at(move.to) = from_piece;

    // Handle promotion
    if (move.is_promotion()) {
        board_.at(move.to).type = move.promotion;
        moving_type = move.promotion;
    }

    // XOR in the piece at its new square
    current_hash_ ^= zobrist_.piece_hash(moving_type, from_piece.color, move.to);

    // Remove from origin
    board_.remove_piece(move.from);

    // En-passant capture: remove the pawn on the adjacent w-layer
    if (move.is_en_passant()) {
        Coord4D to_c = coord_4d(move.to);
        Coord4D from_c = coord_4d(move.from);
        Coord4D ep_pawn = {to_c.x, to_c.y, to_c.z, from_c.w};
        int ep_idx = flat_index(ep_pawn);
        Piece ep_piece = board_.at(ep_idx);
        undo.removed_pieces.push_back({ep_idx, ep_piece});
        current_hash_ ^= zobrist_.piece_hash(ep_piece.type, ep_piece.color, ep_idx);
        board_.remove_piece(ep_idx);
        halfmove_clock_ = 0;
    }

    // Set en-passant target for double pawn pushes
    en_passant_target_ = 255;
    if (from_piece.type == PieceType::PAWN) {
        halfmove_clock_ = 0;
        int w_diff = static_cast<int>(coord_4d(move.to).w) -
                     static_cast<int>(coord_4d(move.from).w);
        if (w_diff == 2 || w_diff == -2) {
            // The en-passant target is the skipped square
            int mid_w = (coord_4d(move.from).w + coord_4d(move.to).w) / 2;
            en_passant_target_ = static_cast<uint8_t>(
                flat_index(coord_4d(move.from).x, coord_4d(move.from).y,
                           coord_4d(move.from).z, mid_w));
        }
    }

    // Toggle side
    current_hash_ ^= zobrist_.side_hash();
    side_to_move_ = (side_to_move_ == Color::WHITE) ? Color::BLACK : Color::WHITE;

    if (side_to_move_ == Color::WHITE) ++fullmove_number_;

    undo_stack_.push(undo);
}

// --- unmake_move ------------------------------------------------------------------
void GameState::unmake_move(const Move& move) {
    if (undo_stack_.empty()) return;
    UndoInfo undo = undo_stack_.top();
    undo_stack_.pop();

    // The piece currently at undo.to is the moved (possibly promoted) piece.
    // Save it before we overwrite the destination with the captured piece.
    Piece moved_piece = board_.at(undo.to);
    if (move.is_promotion()) moved_piece.type = PieceType::PAWN; // revert promotion

    // Restore captured piece at destination
    board_.at(undo.to) = undo.captured;

    // Put the moving piece back at origin
    board_.at(undo.from) = moved_piece;

    // Restore en-passant captured pieces
    for (auto& [idx, piece] : undo.removed_pieces) {
        board_.at(idx) = piece;
    }

    // Restore state
    halfmove_clock_ = undo.halfmove_clock;
    castling_rights_ = undo.castling_rights;
    en_passant_target_ = undo.en_passant_target;
    current_hash_ = undo.hash;
    side_to_move_ = (side_to_move_ == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (side_to_move_ == Color::WHITE) --fullmove_number_;
}

// --- generate_legal_moves ---------------------------------------------------------
int GameState::generate_legal_moves(Move* moves) const {
    return movegen_.generate_legal(board_, side_to_move_, moves);
}

// --- in_check ---------------------------------------------------------------------
bool GameState::in_check() const {
    return movegen_.is_in_check(board_, side_to_move_);
}

// --- is_game_over / checkmate / stalemate -----------------------------------------
bool GameState::is_game_over() const {
    Move moves[MAX_MOVES];
    return movegen_.generate_legal(board_, side_to_move_, moves) == 0;
}

bool GameState::is_checkmate() const {
    if (!in_check()) return false;
    Move moves[MAX_MOVES];
    return movegen_.generate_legal(board_, side_to_move_, moves) == 0;
}

bool GameState::is_stalemate() const {
    if (in_check()) return false;
    Move moves[MAX_MOVES];
    return movegen_.generate_legal(board_, side_to_move_, moves) == 0;
}

// --- trigger_cat_event ------------------------------------------------------------
bool GameState::trigger_cat_event(std::mt19937& rng) {
    if (rng() % 100 >= CAT_PROBABILITY_PERCENT) return false;

    // Find an empty cell for the cat
    std::vector<int> empty_cells;
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (board_.at(i).is_none()) empty_cells.push_back(i);
    }
    if (empty_cells.empty()) return false;

    int cat_cell = empty_cells[rng() % empty_cells.size()];
    Color cat_color = side_to_move_;

    // Save undo info
    UndoInfo undo;
    undo.from = 0; undo.to = 0;
    undo.flags = CAT_SPAWN;
    undo.hash = current_hash_;
    undo.promotion = PieceType::NONE;
    undo.captured = Piece{};
    undo.halfmove_clock = halfmove_clock_;
    undo.castling_rights = castling_rights_;
    undo.en_passant_target = en_passant_target_;

    // Place the cat
    Piece cat = {PieceType::CAT, cat_color};
    board_.at(cat_cell) = cat;
    current_hash_ ^= zobrist_.piece_hash(PieceType::CAT, cat_color, cat_cell);
    undo.spawned_pieces.push_back({cat_cell, cat});

    // Remove all enemy pieces on boundary cells (uses cat_color)
    auto boundary_enemies = movegen_.get_boundary_enemy_cells(board_, cat_color);
    for (int idx : boundary_enemies) {
        Piece removed = board_.at(idx);
        current_hash_ ^= zobrist_.piece_hash(removed.type, removed.color, idx);
        undo.removed_pieces.push_back({idx, removed});
        board_.remove_piece(idx);
    }

    undo_stack_.push(undo);
    return true;
}

// --- trigger_phantom_event --------------------------------------------------------
bool GameState::trigger_phantom_event(std::mt19937& rng) {
    // Phantom only spawns on interior cells (all coords in {1,2}, 16 cells)
    std::vector<int> interior_empty;
    for (int i = 0; i < NUM_CELLS; ++i) {
        if (MoveGenerator::is_boundary(i)) continue;
        if (board_.at(i).is_none()) interior_empty.push_back(i);
    }
    if (interior_empty.empty()) return false;

    int ph_cell = interior_empty[rng() % interior_empty.size()];
    Color ph_color = side_to_move_;

    // Save undo info
    UndoInfo undo;
    undo.from = 0; undo.to = 0;
    undo.flags = PHANTOM_SPAWN;
    undo.hash = current_hash_;
    undo.promotion = PieceType::NONE;
    undo.captured = Piece{};
    undo.halfmove_clock = halfmove_clock_;
    undo.castling_rights = castling_rights_;
    undo.en_passant_target = en_passant_target_;

    Piece phantom = {PieceType::PHANTOM, ph_color};
    board_.at(ph_cell) = phantom;
    current_hash_ ^= zobrist_.piece_hash(PieceType::PHANTOM, ph_color, ph_cell);
    undo.spawned_pieces.push_back({ph_cell, phantom});

    undo_stack_.push(undo);
    return true;
}
