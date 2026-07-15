#include "doctest.h"

#include "view/SpriteLoader.hpp"

TEST_CASE("pieceCodeFromPiece maps all six kinds for white") {
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::King)   == "KW");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::Queen)  == "QW");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::Rook)   == "RW");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::Bishop) == "BW");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::Knight) == "NW");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::White, Kind::Pawn)   == "PW");
}

TEST_CASE("pieceCodeFromPiece maps all six kinds for black") {
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::King)   == "KB");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::Queen)  == "QB");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::Rook)   == "RB");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::Bishop) == "BB");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::Knight) == "NB");
    CHECK(SpriteLoader::pieceCodeFromPiece(Color::Black, Kind::Pawn)   == "PB");
}
