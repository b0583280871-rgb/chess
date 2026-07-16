#pragma once

#include "../ui/img.hpp"
#include "../../backend/engine/GameSnapshot.hpp"


// Draws a single piece's current sprite frame onto the canvas at its
// snapshot pixel position.
void drawPiece(Img& canvas, const PieceSnapshot& piece);

// Draws the board, then every piece in snapshot.pieces, in order.
Img renderFrame(const GameSnapshot& snapshot);
