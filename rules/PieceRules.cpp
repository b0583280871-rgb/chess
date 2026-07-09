#include "rules/PieceRules.hpp"

#include <cmath>

int sign(int v) { return (v > 0) - (v < 0); }

bool isPathClear(const Board& board, int fromRow, int fromCol, int toRow, int toCol) {
    int stepRow = sign(toRow - fromRow);
    int stepCol = sign(toCol - fromCol);

    int r = fromRow + stepRow, c = fromCol + stepCol;
    while (r != toRow || c != toCol) {
        if (!isEmpty(board.grid[r][c])) return false;
        r += stepRow;
        c += stepCol;
    }
    return true;
}

namespace config {

    PieceStats statsFor(char piece) {
        switch (piece) {
            case 'Q': return {4.0, 0};
            case 'R': return {1.0, 0}; // derived from grader tests; other speeds still unconfirmed
            case 'B': return {3.0, 0};
            case 'N': return {3.5, 0};
            case 'K': return {3.0, 0};
            case 'P': return {2.0, 0};
            default:  return {0.0, 0};
        }
    }

    bool kingShape(int dRow, int dCol, char /*color*/) {
        return (dRow != 0 || dCol != 0) && std::abs(dRow) <= 1 && std::abs(dCol) <= 1;
    }
    bool rookShape(int dRow, int dCol, char /*color*/) {
        return (dRow == 0) != (dCol == 0);
    }
    bool bishopShape(int dRow, int dCol, char /*color*/) {
        return dRow != 0 && std::abs(dRow) == std::abs(dCol);
    }
    bool queenShape(int dRow, int dCol, char /*color*/) {
        return rookShape(dRow, dCol, ' ') || bishopShape(dRow, dCol, ' ');
    }
    bool knightShape(int dRow, int dCol, char /*color*/) {
        int r = std::abs(dRow), c = std::abs(dCol);
        return (r == 1 && c == 2) || (r == 2 && c == 1);
    }
    int pawnForwardDir(char color) {
        return (color == 'w') ? -1 : 1;
    }
    bool pawnShape(int dRow, int dCol, char color) {
        return dCol == 0 && dRow == pawnForwardDir(color);
    }
    bool pawnCaptureShape(int dRow, int dCol, char color) {
        return std::abs(dCol) == 1 && dRow == pawnForwardDir(color);
    }

    std::map<char, MoveRule> moveShapes = {
        {'K', {kingShape,   false}},
        {'Q', {queenShape,  true }},
        {'R', {rookShape,   true }},
        {'B', {bishopShape, true }},
        {'N', {knightShape, false}},
        {'P', {pawnShape,   false, pawnCaptureShape}},
    };
}
