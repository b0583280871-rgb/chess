#include "RealTimeArbiter.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "../model/Board.hpp"
#include "../rules/PieceRules.hpp"

namespace {
    constexpr long JUMP_DURATION_MS = 1000;
}

bool RealTimeArbiter::hasActiveMotion() const {
    return activeMove_.has_value();
}

void RealTimeArbiter::startMotion(const PieceMove& move) {
    if (activeMove_.has_value()) throw RealTimeArbiterError("MOTION_ALREADY_ACTIVE");
    activeMove_ = move;
}

bool RealTimeArbiter::hasActiveJump() const {
    return activeJump_.has_value();
}

bool RealTimeArbiter::isPieceCurrentlyMoving(Position pos) const {
    return activeMove_.has_value() && activeMove_->from == pos;
}

bool RealTimeArbiter::isPieceCurrentlyJumping(Position pos) const {
    return activeJump_.has_value() && activeJump_->cell == pos;
}

std::optional<PieceMove> RealTimeArbiter::activeMotionForPiece(Position sourceCell) const {
    if (activeMove_.has_value() && activeMove_->from == sourceCell) {
        return activeMove_;
    }
    return std::nullopt;
}

void RealTimeArbiter::startJump(Position cell, long startMs) {
    if (activeJump_.has_value()) throw RealTimeArbiterError("JUMP_ALREADY_ACTIVE");
    if (isPieceCurrentlyMoving(cell)) throw RealTimeArbiterError("PIECE_ALREADY_MOVING");
    activeJump_ = PieceJump{cell, startMs};
}

ArrivalEvent RealTimeArbiter::advanceTime(long elapsedMs, Board& board) {
    ArrivalEvent event;

    if (activeMove_ && elapsedMs >= activeMove_->startMs + activeMove_->durationMs) {
        const PieceMove m = *activeMove_;
        std::optional<Piece> movingPiece = board.pieceAt(m.from);

        if (movingPiece) {
            bool intercepted = false;

            if (activeJump_ && activeJump_->cell == m.to) {
                std::optional<Piece> jumpingPiece = board.pieceAt(activeJump_->cell);
                if (jumpingPiece && jumpingPiece->color != movingPiece->color) {
                    Piece destroyed = *movingPiece;
                    destroyed.state = PieceState::Captured;
                    event.capturedPiece = destroyed;
                    event.pieceArrived = true;
                    board.removePiece(m.from);
                    activeJump_.reset();  
                    intercepted = true;
                }
            }

            if (!intercepted) {
                std::optional<Piece> destPiece = board.pieceAt(m.to);
                if (!destPiece || destPiece->color != movingPiece->color) {
                    if (destPiece) {
                        destPiece->state = PieceState::Captured;
                        event.capturedPiece = destPiece;
                    }
                    board.movePiece(m.from, m.to);

                    if (config::shouldPromote(*movingPiece, m.to, board.rows())) {
                        board.promoteAt(m.to, config::promotionTarget(*movingPiece));
                    }

                    event.pieceArrived = true;
                }
            }
        }

        activeMove_.reset();
    }

    if (activeJump_ && elapsedMs >= activeJump_->startMs + JUMP_DURATION_MS) {
        activeJump_.reset();   
    }

    return event;
}
