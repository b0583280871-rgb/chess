#pragma once

#include <optional>
#include <utility>

#include "model/Board.hpp"

std::optional<std::pair<int, int>> pixelToCell(int x, int y, const Board& board);
