#pragma once

#include "Board.hpp"
#include "GameState.hpp"

double cellDistance(int r1, int c1, int r2, int c2);

bool isLegalMove(const Board& board, const PieceMove& move, char piece);
