#include "GameEventDetector.hpp"

namespace audio
{

    namespace
    {
        bool hasMovingPiece(const GameSnapshot &snap)
        {
            for (const PieceSnapshot &piece : snap.pieces)
            {
                if (piece.animState == "move")
                    return true;
            }
            return false;
        }
    }

    GameEventDetector::Events GameEventDetector::detect(const GameSnapshot &current)
    {
        if (!previous.has_value())
        {
            previous = current;
            return Events{false, false, false};
        }

        Events events{};

        events.moveStarted = hasMovingPiece(current) && !hasMovingPiece(*previous);

        events.captureHappened = current.pieces.size() < previous->pieces.size();

        events.gameOverJustNow = current.gameOver && !previous->gameOver;

        previous = current;
        return events;
    }

}
