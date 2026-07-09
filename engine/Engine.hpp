#pragma once

#include <string>
#include <vector>

#include "model/GameState.hpp"

void handleClick(GameState& st, int x, int y);

void runCommands(const std::vector<std::string>& commands, GameState& st);
