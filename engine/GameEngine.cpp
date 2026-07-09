#include "engine/GameEngine.hpp"

#include "model/Board.hpp"
#include "rules/PieceRules.hpp"
#include "rules/RuleEngine.hpp"
#include "realtime/RealTimeArbiter.hpp"

void sendMove(GameState& st, int toRow, int toCol) {
    Selection& sel = st.selection;
    const std::string selected = st.board.grid[sel.cell.row][sel.cell.col];

    PieceMove m;
    m.from = sel.cell;
    m.to   = Position{toRow, toCol};
    m.startMs = st.elapsedMs;
    m.piece   = selected;

    char piece = pieceOf(selected);
    double speed = config::statsFor(piece).speedCellsPerSec;
    double dist  = cellDistance(m.from, m.to);
    m.durationMs = (speed > 0.0) ? (long)(dist / speed * 1000.0) : 0;

    if (isLegalMove(st.board, m, piece)) {
        st.board.grid[m.from.row][m.from.col] = ".";
        st.activeMoves.push_back(m);
    }
    sel = Selection{};
}

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    resolveMoves(st);
}
