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

ArrivalEvent RealTimeArbiter::advanceTime(long elapsedMs, Board& board) {
    if (!activeMove_) return ArrivalEvent{};

    const PieceMove& m = *activeMove_;
    if (elapsedMs < m.startMs + m.durationMs) return ArrivalEvent{};   // not yet arrived

    ArrivalEvent event;

    std::optional<Piece> movingPiece = board.pieceAt(m.from);
    if (movingPiece) {
        std::optional<Piece> destPiece = board.pieceAt(m.to);
        if (!destPiece || destPiece->color != movingPiece->color) {
            if (destPiece) {
                destPiece->state = PieceState::Captured;
                event.capturedPiece = destPiece;
            }
            board.movePiece(m.from, m.to);
            event.pieceArrived = true;
        }
    }

    activeMove_.reset();
    return event;
}
