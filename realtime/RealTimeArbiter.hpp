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
    bool isPieceCurrentlyMoving(Position pos) const;   
    bool isPieceCurrentlyJumping(Position pos) const;  
    void startJump(Position cell, long startMs);      

private:
    struct PieceJump {
        Position cell;
        long     startMs;
    };

    std::optional<PieceMove> activeMove_;
    std::optional<PieceJump> activeJump_;
};
