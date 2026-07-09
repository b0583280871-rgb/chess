#pragma once

#include <string>
#include <vector>

#include "Board.hpp"
#include "Position.hpp"

struct Selection {
    bool     active = false;
    Position cell = {0, 0};
    long     selectedAtMs = 0;
};

struct PieceMove {
    Position    from;
    Position    to;
    long        startMs;
    long        durationMs;
    std::string piece;
};

struct GameState {
    Board                  board;
    long                   elapsedMs = 0;
    Selection              selection;
    std::vector<PieceMove> activeMoves;
};
