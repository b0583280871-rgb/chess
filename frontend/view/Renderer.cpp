#include "Renderer.hpp"

#include "SpriteLoader.hpp"
#include "../../backend/rules/config.hpp"


void drawPiece(Img& canvas, const PieceSnapshot& piece) {
    Img& sprite = SpriteLoader::getCachedPieceSprite(piece.pieceCode, piece.animState);
    sprite.draw_on(canvas, piece.pixelX, piece.pixelY);
}

Img renderFrame(const GameSnapshot& snapshot) {
    int width  = snapshot.cols * config::CELL_SIZE;
    int height = snapshot.rows * config::CELL_SIZE;

    Img canvas;
    canvas.read("assets/board.png", {width, height}, false);

    for (const PieceSnapshot& piece : snapshot.pieces) {
        drawPiece(canvas, piece);
    }
    return canvas;
}