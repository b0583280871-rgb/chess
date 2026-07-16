#pragma once

#include <string>

#include "../ui/img.hpp"
#include "../../backend/model/Piece.hpp"

namespace SpriteLoader {

    inline const std::string PIECES_ROOT = "assets/pieces_classic/";

    Img loadPieceIdleSprite(const std::string& pieceCode);

    std::string pieceCodeFromPiece(Color color, Kind kind);

    // Returns a reference to a process-lifetime-cached sprite for
    // (pieceCode, state) - e.g. state "idle" or "move", matching the
    // assets/pieces_classic/<code>/states/<state>/ folder names. Loads and
    // caches on first request per (pieceCode, state) pair; throws
    // std::runtime_error (via Img::read) if the sprite file is missing.
    Img& getCachedPieceSprite(const std::string& pieceCode, const std::string& state);

}
