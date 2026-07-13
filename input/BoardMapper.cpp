#include "BoardMapper.hpp"

#include "../rules/config.hpp"

std::optional<Position> pixelToCell(int x, int y, const Board& board) {
    if (x < 0 || y < 0) return std::nullopt;

    int col = x / config::CELL_SIZE;
    int row = y / config::CELL_SIZE;

    if (row < 0 || row >= board.rows() ||
        col < 0 || col >= board.cols())
        return std::nullopt;

    return Position{row, col};
}
