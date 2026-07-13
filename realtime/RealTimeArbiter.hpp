#pragma once

#include <optional>
#include <stdexcept>
#include <string>

#include "../model/Board.hpp"
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

// Owns the single active motion for the whole board. The common route allows
// only one active motion at a time, globally - not per piece. GameEngine
// consults hasActiveMotion() before starting a new one; startMotion() itself
// throws if that invariant is somehow violated (defensive - should never
// happen given GameEngine's guard).
class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    void startMotion(const PieceMove& move);          // throws RealTimeArbiterError if a motion is already active
    void advanceTime(long elapsedMs, Board& board);    // resolves the active motion's arrival, if due

private:
    std::optional<PieceMove> activeMove_;
};
