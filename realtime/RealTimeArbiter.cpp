#include "RealTimeArbiter.hpp"

#include <cmath>
#include <optional>

#include "../model/Board.hpp"

double cellDistance(Position a, Position b) {
    double dr = b.row - a.row, dc = b.col - a.col;
    return std::sqrt(dr * dr + dc * dc);
}

bool RealTimeArbiter::hasActiveMotion() const {
    return activeMove_.has_value();
}

void RealTimeArbiter::startMotion(const PieceMove& move) {
    if (activeMove_.has_value()) throw RealTimeArbiterError("MOTION_ALREADY_ACTIVE");
    activeMove_ = move;
}

void RealTimeArbiter::advanceTime(long elapsedMs, Board& board) {
    if (!activeMove_) return;

    const PieceMove& m = *activeMove_;
    if (elapsedMs < m.startMs + m.durationMs) return;   // not yet arrived

    std::optional<Piece> movingPiece = board.pieceAt(m.from);
    if (movingPiece) {
        std::optional<Piece> destPiece = board.pieceAt(m.to);
        if (!destPiece || destPiece->color != movingPiece->color) {
            board.movePiece(m.from, m.to);
        }
    }

    activeMove_.reset();
}
