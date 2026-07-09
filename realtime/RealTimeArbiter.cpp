#include "realtime/RealTimeArbiter.hpp"

#include <algorithm>
#include <cmath>

#include "model/Board.hpp"

double cellDistance(int r1, int c1, int r2, int c2) {
    double dr = r2 - r1, dc = c2 - c1;
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
        std::string& target = st.board.grid[m.toRow][m.toCol];
        if (isEmpty(target) || colorOf(target) != m.piece[0]) {
            target = m.piece;
        } else {
            std::string& origin = st.board.grid[m.fromRow][m.fromCol];
            if (isEmpty(origin)) origin = m.piece;
        }
    }

    st.activeMoves = stillMoving;
}
