#include "engine/GameEngine.hpp"

#include "model/Board.hpp"
#include "rules/PieceRules.hpp"
#include "rules/RuleEngine.hpp"
#include "realtime/RealTimeArbiter.hpp"

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

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    resolveMoves(st);
}
