#pragma once

#include <functional>
#include <map>

#include "model/Board.hpp"

int sign(int v);

bool isPathClear(const Board& board, int fromRow, int fromCol, int toRow, int toCol);

namespace config {

    struct PieceStats {
        double speedCellsPerSec;
        long   restMs;
    };

    PieceStats statsFor(char piece);

    using MoveShapeFn = std::function<bool(int dRow, int dCol, char color)>;

    bool kingShape(int dRow, int dCol, char color);
    bool rookShape(int dRow, int dCol, char color);
    bool bishopShape(int dRow, int dCol, char color);
    bool queenShape(int dRow, int dCol, char color);
    bool knightShape(int dRow, int dCol, char color);
    int  pawnForwardDir(char color);
    bool pawnShape(int dRow, int dCol, char color);
    bool pawnCaptureShape(int dRow, int dCol, char color);

    struct MoveRule {
        MoveShapeFn shape;
        bool        slides;
        MoveShapeFn captureShape = nullptr;
    };

    extern std::map<char, MoveRule> moveShapes;
}
