#include "SpriteLoader.hpp"

namespace SpriteLoader {

    Img loadPieceIdleSprite(const std::string& pieceCode) {
        Img img;
        img.read(PIECES_ROOT + pieceCode + "/states/idle/sprites/1.png");
        return img;
    }

    std::string pieceCodeFromPiece(Color color, Kind kind) {
        char kindChar = 'K';
        switch (kind) {
            case Kind::King:   kindChar = 'K'; break;
            case Kind::Queen:  kindChar = 'Q'; break;
            case Kind::Rook:   kindChar = 'R'; break;
            case Kind::Bishop: kindChar = 'B'; break;
            case Kind::Knight: kindChar = 'N'; break;
            case Kind::Pawn:   kindChar = 'P'; break;
        }
        char colorChar = (color == Color::White) ? 'W' : 'B';
        return std::string{kindChar, colorChar};
    }

}
