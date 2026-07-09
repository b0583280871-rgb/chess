#include "Board.hpp"

bool isEmpty(const std::string& tok) { return tok == "."; }
char colorOf(const std::string& tok) { return tok[0]; }
char pieceOf(const std::string& tok) { return tok[1]; }

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
