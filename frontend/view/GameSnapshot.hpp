#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../../backend/model/GameState.hpp"

struct PieceSnapshot {
    std::string pieceCode;    
    int pixelX, pixelY;        
    std::string animState;   
};

struct GameSnapshot {
    int rows, cols;
    std::vector<PieceSnapshot> pieces;
    std::optional<Position> selectedCell;   
    bool gameOver;
};

GameSnapshot buildSnapshot(const GameState& state);
