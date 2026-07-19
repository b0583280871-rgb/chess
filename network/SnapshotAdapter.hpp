#pragma once

#include "../backend/engine/GameSnapshot.hpp"
#include "protocol/Messages.hpp"

// Pure conversion between the engine's GameSnapshot/PieceSnapshot and the
// wire-level protocol::SnapshotMessage/SnapshotPieceMessage. network/ is
// allowed to depend on engine/ (this file does, deliberately) - the reverse
// must never happen: engine/GameSnapshot.hpp has no include of anything
// under network/, and nothing in engine/ knows this file exists.
namespace SnapshotAdapter {

    inline protocol::SnapshotMessage toProtocol(const GameSnapshot& snap) {
        protocol::SnapshotMessage msg;
        msg.rows = snap.rows;
        msg.cols = snap.cols;
        msg.gameOver = snap.gameOver;

        msg.pieces.reserve(snap.pieces.size());
        for (const PieceSnapshot& piece : snap.pieces) {
            msg.pieces.push_back(protocol::SnapshotPieceMessage{
                piece.pieceCode, piece.pixelX, piece.pixelY, piece.animState, piece.frameIndex
            });
        }

        if (snap.selectedCell) {
            msg.selectedCell = protocol::SnapshotCell{snap.selectedCell->row, snap.selectedCell->col};
        }

        return msg;
    }

    inline GameSnapshot fromProtocol(const protocol::SnapshotMessage& msg) {
        GameSnapshot snap;
        snap.rows = msg.rows;
        snap.cols = msg.cols;
        snap.gameOver = msg.gameOver;

        snap.pieces.reserve(msg.pieces.size());
        for (const protocol::SnapshotPieceMessage& piece : msg.pieces) {
            PieceSnapshot ps;
            ps.pieceCode = piece.pieceCode;
            ps.pixelX = piece.pixelX;
            ps.pixelY = piece.pixelY;
            ps.animState = piece.animState;
            ps.frameIndex = piece.frameIndex;
            snap.pieces.push_back(ps);
        }

        if (msg.selectedCell) {
            snap.selectedCell = Position{msg.selectedCell->row, msg.selectedCell->col};
        }

        return snap;
    }

}
