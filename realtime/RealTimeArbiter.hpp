#pragma once

#include "model/GameState.hpp"
#include "model/Position.hpp"

double cellDistance(Position a, Position b);

void resolveMoves(GameState& st);
