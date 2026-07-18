#include "GameSnapshot.hpp"

#include <algorithm>

#include "GameEngine.hpp"
#include "../rules/config.hpp"

namespace {
    std::string pieceCodeFromPiece(Color color, Kind kind) {
        char kindChar = 'K';
        switch (kind) {
            case Kind::King:   kindChar = 'K'; break;
            case Kind::Queen:  kindChar = 'Q'; break;
            case Kind::Rook:   kindChar = 'R'; break;
            case Kind::Bishop: kindChar = 'B'; break;
            case Kind::Knight: kindChar = 'N'; break;
            case Kind::Pawn:   kindChar = 'P'; break;
        }
        char colorChar = (color == Color::White) ? 'W' : 'B';
        return std::string{kindChar, colorChar};
    }
}

namespace GameEngine {

    GameSnapshot snapshot(const GameState& state) {
        GameSnapshot snap;
        snap.rows = state.board.rows();
        snap.cols = state.board.cols();
        snap.gameOver = state.gameOver;

        for (int row = 0; row < state.board.rows(); ++row) {
            for (int col = 0; col < state.board.cols(); ++col) {
                Position pos{row, col};
                std::optional<Piece> piece = state.board.pieceAt(pos);
                if (!piece) continue;
                if (piece->state == PieceState::Captured) continue; 

                PieceSnapshot ps;
                ps.pieceCode = pieceCodeFromPiece(piece->color, piece->kind);

                std::optional<PieceMove> activeMotion = state.arbiter.activeMotionForPiece(piece->cell);
                if (activeMotion) {
                    double progress = activeMotion->durationMs > 0
                        ? (double)(state.elapsedMs - activeMotion->startMs) / activeMotion->durationMs
                        : 1.0;
                    progress = std::clamp(progress, 0.0, 1.0);

                    int fromPixelX = activeMotion->from.col * config::CELL_SIZE;
                    int fromPixelY = activeMotion->from.row * config::CELL_SIZE;
                    int toPixelX   = activeMotion->to.col   * config::CELL_SIZE;
                    int toPixelY   = activeMotion->to.row   * config::CELL_SIZE;

                    ps.pixelX = fromPixelX + (int)((toPixelX - fromPixelX) * progress);
                    ps.pixelY = fromPixelY + (int)((toPixelY - fromPixelY) * progress);
                } else {
                    ps.pixelX = pos.col * config::CELL_SIZE;
                    ps.pixelY = pos.row * config::CELL_SIZE;
                }

                ps.animState = state.arbiter.isPieceCurrentlyMoving(piece->cell) ? "move" : "idle";

                snap.pieces.push_back(ps);
            }
        }

        if (state.selection.active) {
            snap.selectedCell = state.selection.cell;
        }

        return snap;
    }

}
