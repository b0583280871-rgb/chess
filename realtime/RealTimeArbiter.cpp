#include "RealTimeArbiter.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "../model/Board.hpp"
#include "../rules/PieceRules.hpp"

double cellDistance(Position a, Position b) {
    double dr = std::abs(b.row - a.row);
    double dc = std::abs(b.col - a.col);
    return std::max(dr, dc);
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

            if (movingPiece->kind == Kind::Pawn) {
                int lastRow = (config::pawnForwardDir(colorToChar(movingPiece->color)) < 0) ? 0 : board.rows() - 1;
                if (m.to.row == lastRow) {
                    board.promoteAt(m.to, Kind::Queen);
                }
            }

            event.pieceArrived = true;
        }
    }

    activeMove_.reset();
    return event;
}
