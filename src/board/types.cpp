#include "types.h"
#include <sstream>
#include <iomanip>

std::string Move::to_string() const {
    Coord4D fc = coord_4d(this->from);
    Coord4D tc = coord_4d(this->to);
    std::ostringstream ss;
    ss << "(" << fc.x << "," << fc.y << "," << fc.z << "," << fc.w << ")"
       << "->("
       << tc.x << "," << tc.y << "," << tc.z << "," << tc.w << ")";
    if (is_capture()) ss << "x";
    if (is_promotion()) {
        ss << "=PROMO";
        switch (promotion) {
            case PieceType::QUEEN:  ss << "Q"; break;
            case PieceType::ROOK:   ss << "R"; break;
            case PieceType::BISHOP: ss << "B"; break;
            case PieceType::KNIGHT: ss << "N"; break;
            default: break;
        }
    }
    if (is_cat_spawn()) ss << "[CAT]";
    if (is_phantom_spawn()) ss << "[PHANTOM]";
    if (is_castling()) ss << "[CASTLE]";
    if (is_en_passant()) ss << "[EP]";
    return ss.str();
}
