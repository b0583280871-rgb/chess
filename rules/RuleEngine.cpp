#include "rules/RuleEngine.hpp"

#include "rules/PieceRules.hpp"

bool isLegalMove(const Board& board, const PieceMove& move, char piece) {
    auto it = config::moveShapes.find(piece);
    if (it == config::moveShapes.end()) return true;   // no rule registered yet -> unrestricted for now

    const config::MoveRule& rule = it->second;
    char color = move.piece[0];

    const std::string &destination = board.grid[move.to.row][move.to.col];
    if (!isEmpty(destination) && colorOf(destination) == color) return false;

    bool isCapture = !isEmpty(destination);
    const config::MoveShapeFn& shape = (isCapture && rule.captureShape) ? rule.captureShape : rule.shape;

    int dRow = move.to.row - move.from.row;
    int dCol = move.to.col - move.from.col;
    if (!shape(dRow, dCol, color)) return false;

    if (rule.slides && !isPathClear(board, move.from, move.to))
        return false;

    return true;
}
