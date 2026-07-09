#include "Engine.hpp"

#include <iostream>
#include <sstream>

#include "model/Board.hpp"
#include "rules/config.hpp"
#include "rules/PieceRules.hpp"
#include "rules/RuleEngine.hpp"
#include "realtime/RealTimeArbiter.hpp"
#include "io/BoardParser.hpp"
#include "io/BoardPrinter.hpp"

void sendMove(GameState& st, int toRow, int toCol) {
    Selection& sel = st.selection;
    const std::string selected = st.board.grid[sel.row][sel.col];

    PieceMove m;
    m.fromRow = sel.row; m.fromCol = sel.col;
    m.toRow = toRow;     m.toCol = toCol;
    m.startMs = st.elapsedMs;
    m.piece   = selected;

    char piece = pieceOf(selected);
    double speed = config::statsFor(piece).speedCellsPerSec;
    double dist  = cellDistance(m.fromRow, m.fromCol, toRow, toCol);
    m.durationMs = (speed > 0.0) ? (long)(dist / speed * 1000.0) : 0;

    if (isLegalMove(st.board, m, piece)) {
        st.board.grid[m.fromRow][m.fromCol] = ".";
        st.activeMoves.push_back(m);
    }
    sel = Selection{};
}

void handleClick(GameState& st, int x, int y) {
    if (x < 0 || y < 0) return;

    int col = x / config::CELL_SIZE;
    int row = y / config::CELL_SIZE;

    if (row < 0 || row >= st.board.rows() ||
        col < 0 || col >= st.board.cols())
        return;

    const std::string& token = st.board.grid[row][col];

    if (st.selection.active) {
        const std::string& selectedTok = st.board.grid[st.selection.row][st.selection.col];
        bool sameSide = !isEmpty(token) && colorOf(token) == colorOf(selectedTok);
        if (sameSide) {
            st.selection = {true, row, col, st.elapsedMs};   // reselect
        } else {
            sendMove(st, row, col);                           // complete: move or capture
        }
        return;
    }

    if (!isEmpty(token)) {
        st.selection = {true, row, col, st.elapsedMs};       // open a fresh selection
    }
}

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    resolveMoves(st);
}

void runCommands(const std::vector<std::string>& commands, GameState& st) {
    for (const auto& command : commands) {
        std::istringstream ss(command);
        std::string verb;
        ss >> verb;

        if (verb == "click") {
            int x, y;
            ss >> x >> y;
            handleClick(st, x, y);
        } else if (verb == "wait") {
            long ms; ss >> ms;
            handleWait(st, ms);
        } else if (verb == "print") {
            std::string rest; std::getline(ss, rest);
            if (trim(rest) == "board") std::cout << formatBoard(st.board);
        }
    }
}
