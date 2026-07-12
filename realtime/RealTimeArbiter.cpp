#include "realtime/RealTimeArbiter.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "model/Board.hpp"

double cellDistance(Position a, Position b) {
    double dr = b.row - a.row, dc = b.col - a.col;
    return std::sqrt(dr * dr + dc * dc);
}

void resolveMoves(GameState& st) {
    std::vector<size_t>    due;
    std::vector<PieceMove> stillMoving;
    for (size_t i = 0; i < st.activeMoves.size(); ++i) {
        const PieceMove& m = st.activeMoves[i];
        if (st.elapsedMs >= m.startMs + m.durationMs) due.push_back(i);
        else                                          stillMoving.push_back(m);
    }

    std::sort(due.begin(), due.end(), [&](size_t a, size_t b) {
        long ta = st.activeMoves[a].startMs + st.activeMoves[a].durationMs;
        long tb = st.activeMoves[b].startMs + st.activeMoves[b].durationMs;
        if (ta != tb) return ta < tb;
        return a < b;
    });

    for (size_t idx : due) {
        const PieceMove& m = st.activeMoves[idx];

        std::optional<Piece> movingPiece = st.board.pieceAt(m.from);
        if (!movingPiece) continue;   // defensive: shouldn't normally happen

        std::optional<Piece> destPiece = st.board.pieceAt(m.to);
        if (!destPiece || destPiece->color != movingPiece->color) {
            st.board.movePiece(m.from, m.to);
        }
        // else: destination holds a friendly piece -> the move is blocked.
        // No "restore to origin" is needed here: GameEngine no longer clears
        // the source cell when the move is sent, so the piece was never
        // removed from m.from in the first place - it simply stays put.
    }

    st.activeMoves = stillMoving;
}
