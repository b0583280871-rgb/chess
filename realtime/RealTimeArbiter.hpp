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

struct ArrivalEvent {
    bool pieceArrived = false;
    std::optional<Piece> capturedPiece;   
};

class RealTimeArbiter {
public:
    bool hasActiveMotion() const;
    void startMotion(const PieceMove& move);
    ArrivalEvent advanceTime(long elapsedMs, Board& board);

    bool hasActiveJump() const;
    bool isPieceCurrentlyMoving(Position pos) const;   // true if activeMove_ exists and its `from` equals pos
    bool isPieceCurrentlyJumping(Position pos) const;  // true if activeJump_ exists and its cell equals pos
    void startJump(Position cell, long startMs);       // throws RealTimeArbiterError if a jump is already active, or the piece is currently moving

private:
    // PieceJump is a private implementation detail of RealTimeArbiter alone -
    // nothing outside this class ever needs to name its type. It's declared
    // as a private nested struct (rather than an anonymous-namespace type in
    // the .cpp) because std::optional<PieceJump> as a member here requires a
    // complete type visible at this point in the header; an anonymous
    // namespace in RealTimeArbiter.cpp is invisible from this header entirely.
    struct PieceJump {
        Position cell;
        long     startMs;
    };

    std::optional<PieceMove> activeMove_;
    std::optional<PieceJump> activeJump_;
};
