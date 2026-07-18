#pragma once

#include "../ui/img.hpp"
#include "../../backend/engine/GameSnapshot.hpp"


// Draws a single piece's current sprite frame onto the canvas at its
// snapshot pixel position.
void drawPiece(Img& canvas, const PieceSnapshot& piece);

// Draws a visual highlight (a colored border) around the cell at
// selectedCell, if it has a value. Does nothing if selectedCell is
// std::nullopt.
void drawSelection(Img& canvas, const std::optional<Position>& selectedCell);

// Draws a game-over message centered (roughly) on the canvas. Only called
// when the snapshot's gameOver flag is true.
void drawGameOver(Img& canvas, int canvasWidth, int canvasHeight);

// Draws the board, then every piece in snapshot.pieces, in order.
Img renderFrame(const GameSnapshot& snapshot);
