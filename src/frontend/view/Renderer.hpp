#pragma once

#include "../ui/img.hpp"
#include "../../backend/engine/GameSnapshot.hpp"


void drawPiece(Img& canvas, const PieceSnapshot& piece);

void drawSelection(Img& canvas, const std::optional<Position>& selectedCell);

void drawGameOver(Img& canvas, int canvasWidth, int canvasHeight);

Img renderFrame(const GameSnapshot& snapshot);
