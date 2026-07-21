#include "Renderer.hpp"

#include "SpriteLoader.hpp"
#include "../../backend/rules/config.hpp"

constexpr std::string_view BACKGROUND_IMG_BOUARD_PATH = "assets/background.jpg";
const cv::Scalar SELECTION_HIGHLIGHT_COLOR(0, 0, 255, 255);
const cv::Scalar GAME_OVER_TEXT_COLOR(0, 255, 255, 255);
const cv::Scalar GAME_OVER_BACKGROUND_COLOR(20, 20, 20, 255);

void drawPiece(Img &canvas, const PieceSnapshot &piece)
{
    Img &sprite = SpriteLoader::getCachedPieceFrame(piece.pieceCode, piece.animState, piece.frameIndex);
    sprite.draw_on(canvas, piece.pixelX, piece.pixelY);
}

void drawSelection(Img &canvas, const std::optional<Position> &selectedCell)
{
    if (!selectedCell)
        return;

    int x = selectedCell->col * config::CELL_SIZE;
    int y = selectedCell->row * config::CELL_SIZE;
    const int thickness = 4;

    Img top, bottom, left, right;
    top.create(config::CELL_SIZE, thickness, SELECTION_HIGHLIGHT_COLOR);
    bottom.create(config::CELL_SIZE, thickness, SELECTION_HIGHLIGHT_COLOR);
    left.create(thickness, config::CELL_SIZE, SELECTION_HIGHLIGHT_COLOR);
    right.create(thickness, config::CELL_SIZE, SELECTION_HIGHLIGHT_COLOR);

    top.draw_on(canvas, x, y);
    bottom.draw_on(canvas, x, y + config::CELL_SIZE - thickness);
    left.draw_on(canvas, x, y);
    right.draw_on(canvas, x + config::CELL_SIZE - thickness, y);
}

void drawGameOver(Img &canvas, int canvasWidth, int canvasHeight)
{
    const std::string text = "GAME OVER";
    const double fontScale = 1.8;
    const int thickness = 3;

    int baseline = 0;
    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);

    int textX = (canvasWidth - textSize.width) / 2;
    int textY = (canvasHeight + textSize.height) / 2;

    const int paddingX = 24;
    const int paddingY = 16;
    int boxWidth = textSize.width + 2 * paddingX;
    int boxHeight = textSize.height + baseline + 2 * paddingY;
    int boxX = (canvasWidth - boxWidth) / 2;
    int boxY = textY - textSize.height - paddingY;

    Img background;
    background.create(boxWidth, boxHeight, GAME_OVER_BACKGROUND_COLOR);
    background.draw_on(canvas, boxX, boxY);

    canvas.put_text(text, textX, textY, fontScale, GAME_OVER_TEXT_COLOR, thickness);
}

Img renderFrame(const GameSnapshot &snapshot)
{
    int width = snapshot.cols * config::CELL_SIZE;
    int height = snapshot.rows * config::CELL_SIZE;

    Img canvas;
    canvas.read(BACKGROUND_IMG_BOUARD_PATH.data(), {width, height}, false);

    for (const PieceSnapshot &piece : snapshot.pieces)
    {
        drawPiece(canvas, piece);
    }

    drawSelection(canvas, snapshot.selectedCell);

    if (snapshot.gameOver)
    {
        drawGameOver(canvas, width, height);
    }

    return canvas;
}