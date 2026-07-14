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
                    // Interception: the arriving enemy is destroyed mid-air,
                    // never placed at the destination; the jumper is untouched.
                    Piece destroyed = *movingPiece;
                    destroyed.state = PieceState::Captured;
                    event.capturedPiece = destroyed;
                    event.pieceArrived = true;
                    board.removePiece(m.from);
                    activeJump_.reset();   // the jump concluded successfully
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
                // else: friendly-blocked - event.pieceArrived stays false,
                // board unchanged, and activeJump_ (if any) is left untouched.
            }
        }

        activeMove_.reset();
    }

    // Step B: separately, resolve the jump's own timeout - only reached here
    // if it wasn't already concluded by interception in Step A above (in
    // which case activeJump_ is already reset and this is a no-op).
    if (activeJump_ && elapsedMs >= activeJump_->startMs + JUMP_DURATION_MS) {
        activeJump_.reset();   // lands normally: zero board change
    }

    return event;
}
