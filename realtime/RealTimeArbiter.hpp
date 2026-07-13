#pragma once

#include <optional>
#include <stdexcept>
#include <string>

#include "../model/Board.hpp"
#include "../model/Piece.hpp"
#include "../model/PieceMove.hpp"
#include "../model/Position.hpp"

double cellDistance(Position a, Position b);

class RealTimeArbiterError : public std::runtime_error {
public:
    explicit RealTimeArbiterError(const std::string& code)
        : std::runtime_error(code), code_(code) {}
    const std::string& code() const { return code_; }
private:
    std::string code_;
};

// Reports the facts of an advanceTime() call - whether the active motion
// arrived this tick, and which piece (if any) was captured on arrival.
// RealTimeArbiter only reports what happened; it does not interpret what a
// capture means for the game (e.g. a king capture ending the game) - that
// interpretation belongs to GameEngine.
struct ArrivalEvent {
    bool pieceArrived = false;
    std::optional<Piece> capturedPiece;   // set only if pieceArrived is true AND a piece was captured on arrival
};

// Owns the single active motion for the whole board. The common route allows
// only one active motion at a time, globally - not per piece. GameEngine
// consults hasActiveMotion() before starting a new one; startMotion() itself
// throws if that invariant is somehow violated (defensive - should never
// happen given GameEngine's guard).
class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    void startMotion(const PieceMove& move);                    // throws RealTimeArbiterError if a motion is already active
    ArrivalEvent advanceTime(long elapsedMs, Board& board);      // resolves the active motion's arrival, if due

private:
    std::optional<PieceMove> activeMove_;
};
