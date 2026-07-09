#pragma once

#include "model/GameState.hpp"

double cellDistance(int r1, int c1, int r2, int c2);

void resolveMoves(GameState& st);
