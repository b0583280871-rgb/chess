#pragma once

#include <string>

#include "../ui/img.hpp"
#include "../../backend/model/Piece.hpp"

namespace SpriteLoader {

    inline const std::string PIECES_ROOT = "assets/pieces2/";

    Img loadPieceIdleSprite(const std::string& pieceCode);

    std::string pieceCodeFromPiece(Color color, Kind kind);

}
