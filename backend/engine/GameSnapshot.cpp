#include "GameSnapshot.hpp"

#include "GameEngine.hpp"
#include "../rules/config.hpp"

//i have to move to current place and switch to the map
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

                ps.pixelX = pos.col * config::CELL_SIZE;
                ps.pixelY = pos.row * config::CELL_SIZE;

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
