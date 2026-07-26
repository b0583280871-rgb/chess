#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../model/GameState.hpp"

struct PieceSnapshot {
    std::string pieceCode;
    int pixelX, pixelY;
    std::string animState;
    int frameIndex;      // 1-based sprite frame to display for animState
};

struct GameSnapshot {
    int rows, cols;
    std::vector<PieceSnapshot> pieces;
    std::optional<Position> selectedCell;
    bool gameOver;
};

namespace GameEngine {
    int computeAnimationFrameIndex(long phaseMs, int framesPerSec, int frameCount, bool isLoop);
}
