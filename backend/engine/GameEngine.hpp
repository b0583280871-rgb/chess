#pragma once

#include "../model/GameState.hpp"
#include "GameSnapshot.hpp"

void sendMove(GameState& st, int toRow, int toCol);

bool startJump(GameState& st, Position cell);

void handleWait(GameState& st, long ms);

namespace GameEngine {
    GameSnapshot snapshot(const GameState& state);
}
