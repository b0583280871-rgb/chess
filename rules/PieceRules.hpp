#pragma once

#include <functional>
#include <map>

#include "model/Board.hpp"
#include "model/Piece.hpp"
#include "model/Position.hpp"

int sign(int v);

bool isPathClear(const Board& board, Position from, Position to);

namespace config {

    struct PieceStats {
        double speedCellsPerSec;
        long   restMs;
    };

    PieceStats statsFor(Kind kind);

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

    extern std::map<Kind, MoveRule> moveShapes;
}
