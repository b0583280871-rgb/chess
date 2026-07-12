#include "model/Piece.hpp"

#include "model/Board.hpp"

bool isValidKindChar(char c) {
    switch (c) {
        case 'K': case 'Q': case 'R':
        case 'B': case 'N': case 'P': return true;
        default: return false;
    }
}

Kind kindFromChar(char c) {
    switch (c) {
        case 'K': return Kind::King;
        case 'Q': return Kind::Queen;
        case 'R': return Kind::Rook;
        case 'B': return Kind::Bishop;
        case 'N': return Kind::Knight;
        case 'P': return Kind::Pawn;
        default:  throw PieceError("INVALID_PIECE_TOKEN");
    }
}

char charFromKind(Kind k) {
    switch (k) {
        case Kind::King:   return 'K';
        case Kind::Queen:  return 'Q';
        case Kind::Rook:   return 'R';
        case Kind::Bishop: return 'B';
        case Kind::Knight: return 'N';
        case Kind::Pawn:   return 'P';
    }
    throw PieceError("INVALID_PIECE_TOKEN");
}

namespace {
    bool isValidPieceToken(const std::string& token) {
        if (token.size() != 2) return false;
        if (token[0] != 'w' && token[0] != 'b') return false;
        return isValidKindChar(token[1]);
    }
}

Piece pieceFromToken(const std::string& token, Position at, int id) {
    if (!isValidPieceToken(token)) throw PieceError("INVALID_PIECE_TOKEN");

    Piece piece;
    piece.id    = id;
    piece.color = (colorOf(token) == 'w') ? Color::White : Color::Black;
    piece.kind  = kindFromChar(pieceOf(token));
    piece.cell  = at;
    piece.state = PieceState::Idle;
    return piece;
}

std::string tokenFromPiece(const Piece& piece) {
    std::string token(2, '?');
    token[0] = (piece.color == Color::White) ? 'w' : 'b';
    token[1] = charFromKind(piece.kind);
    return token;
}
