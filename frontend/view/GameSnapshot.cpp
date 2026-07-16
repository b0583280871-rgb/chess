#include "GameSnapshot.hpp"

#include "SpriteLoader.hpp"
#include "../../backend/rules/config.hpp"

GameSnapshot buildSnapshot(const GameState& state) {
    GameSnapshot snapshot;
    snapshot.rows = state.board.rows();
    snapshot.cols = state.board.cols();
    snapshot.gameOver = state.gameOver;

    for (int row = 0; row < state.board.rows(); ++row) {
        for (int col = 0; col < state.board.cols(); ++col) {
            Position pos{row, col};
            std::optional<Piece> piece = state.board.pieceAt(pos);
            if (!piece) continue;
            if (piece->state == PieceState::Captured) continue;   

            PieceSnapshot ps;
            ps.pieceCode = SpriteLoader::pieceCodeFromPiece(piece->color, piece->kind);

            ps.pixelX = pos.col * config::CELL_SIZE;
            ps.pixelY = pos.row * config::CELL_SIZE;

            ps.animState = state.arbiter.isPieceCurrentlyMoving(piece->cell) ? "move" : "idle";

            snapshot.pieces.push_back(ps);
        }
    }

    if (state.selection.active) {
        snapshot.selectedCell = state.selection.cell;
    }

    return snapshot;
}
