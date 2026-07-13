#include "GameEngine.hpp"

#include <optional>

#include "../model/Board.hpp"
#include "../rules/PieceRules.hpp"
#include "../rules/RuleEngine.hpp"
#include "../realtime/RealTimeArbiter.hpp"

void sendMove(GameState& st, int toRow, int toCol) {
    Selection& sel = st.selection;

    if (st.gameOver) {
        sel = Selection{};
        return;
    }
    if (st.arbiter.hasActiveMotion()) {
        sel = Selection{};
        return;
    }
    if (st.arbiter.isPieceCurrentlyJumping(sel.cell)) {
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

void startJump(GameState& st, Position cell) {
    if (st.gameOver) return;

    std::optional<Piece> piece = st.board.pieceAt(cell);
    if (!piece) return;

    try {
        st.arbiter.startJump(cell, st.elapsedMs);
    } catch (const RealTimeArbiterError&) {
        // Silently rejected - same treatment as any other illegal/blocked
        // move attempt elsewhere in this codebase: no error channel, no
        // exception escapes GameEngine.
    }
}

void handleWait(GameState& st, long ms) {
    st.elapsedMs += ms;
    ArrivalEvent event = st.arbiter.advanceTime(st.elapsedMs, st.board);
    if (event.capturedPiece && event.capturedPiece->kind == Kind::King) {
        st.gameOver = true;
    }
}
