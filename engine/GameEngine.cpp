#include "GameEngine.hpp"

#include <optional>

#include "../model/Board.hpp"
#include "../rules/PieceRules.hpp"
#include "../rules/RuleEngine.hpp"
#include "../realtime/RealTimeArbiter.hpp"

void sendMove(GameState& st, int toRow, int toCol) {
    Selection& sel = st.selection;

    if (st.arbiter.hasActiveMotion()) {
        sel = Selection{};
        return;
    }

    std::optional<Piece> movingPiece = st.board.pieceAt(sel.cell);
    if (!movingPiece) {
        sel = Selection{};
        return;
    }
    const std::string selected = tokenFromPiece(*movingPiece);

    PieceMove m;
    m.from = sel.cell;
    m.to   = Position{toRow, toCol};
    m.startMs = st.elapsedMs;
    m.piece   = selected;

    char piece = pieceOf(selected);
    Kind kind = kindFromChar(piece);
    double speed = config::statsFor(kind).speedCellsPerSec;
    double dist  = cellDistance(m.from, m.to);
    m.durationMs = (speed > 0.0) ? (long)(dist / speed * 1000.0) : 0;

    if (isLegalMove(st.board, m, piece)) {
        st.arbiter.startMotion(m);
    }
    sel = Selection{};
}

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    st.arbiter.advanceTime(st.elapsedMs, st.board);
}
