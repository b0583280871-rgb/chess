#include "GameSnapshot.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

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

    const std::string PIECES_ROOT = "assets/pieces_classic/";

    struct AnimationConfig {
        int framesPerSec = 1;
        bool isLoop = true;
        int frameCount = 1;
    };

    std::string extractRawValue(const std::string& json, const std::string& key) {
        std::string needle = "\"" + key + "\"";
        size_t keyPos = json.find(needle);
        if (keyPos == std::string::npos) {
            throw std::runtime_error("Missing key '" + key + "' in animation config");
        }
        size_t colonPos = json.find(':', keyPos + needle.size());
        size_t valueStart = colonPos + 1;
        while (valueStart < json.size() && std::isspace((unsigned char)json[valueStart])) ++valueStart;

        if (valueStart < json.size() && json[valueStart] == '"') {
            size_t valueEnd = json.find('"', valueStart + 1);
            return json.substr(valueStart + 1, valueEnd - valueStart - 1);
        }

        size_t valueEnd = valueStart;
        while (valueEnd < json.size() &&
               json[valueEnd] != ',' && json[valueEnd] != '}' &&
               !std::isspace((unsigned char)json[valueEnd])) {
            ++valueEnd;
        }
        return json.substr(valueStart, valueEnd - valueStart);
    }

    int countSpriteFrames(const std::string& pieceCode, const std::string& state) {
        int count = 0;
        while (true) {
            std::string path = PIECES_ROOT + pieceCode + "/states/" + state +
                                "/sprites/" + std::to_string(count + 1) + ".png";
            if (!std::filesystem::exists(path)) break;
            ++count;
        }
        return count;
    }

    AnimationConfig loadAnimationConfig(const std::string& pieceCode, const std::string& state) {
        std::string configPath = PIECES_ROOT + pieceCode + "/states/" + state + "/config.json";
        std::ifstream file(configPath);
        if (!file) {
            throw std::runtime_error("Cannot open animation config: " + configPath);
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();

        AnimationConfig cfg;
        cfg.framesPerSec = std::stoi(extractRawValue(json, "frames_per_sec"));
        std::string loopRaw = extractRawValue(json, "is_loop");
        cfg.isLoop = (loopRaw == "true" || loopRaw == "1");

        cfg.frameCount = countSpriteFrames(pieceCode, state);
        if (cfg.frameCount < 1) {
            throw std::runtime_error("No sprite frames found for " + pieceCode + "/" + state);
        }

        return cfg;
    }

    const AnimationConfig& cachedAnimationConfig(const std::string& pieceCode, const std::string& state) {
        static std::unordered_map<std::string, AnimationConfig> cache;

        const std::string key = pieceCode + "_" + state;
        auto it = cache.find(key);
        if (it != cache.end()) return it->second;

        return cache.emplace(key, loadAnimationConfig(pieceCode, state)).first->second;
    }
}

namespace GameEngine {

    int computeAnimationFrameIndex(long phaseMs, int framesPerSec, int frameCount, bool isLoop) {
        if (phaseMs < 0) phaseMs = 0;
        int rawFrame = (int)(phaseMs / 1000.0 * framesPerSec);
        if (isLoop) {
            return 1 + (rawFrame % frameCount);
        }
        return 1 + std::min(rawFrame, frameCount - 1);
    }

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

                long phaseStartMs = (ps.animState == "move" && activeMotion) ? activeMotion->startMs : 0;
                long phaseMs = state.elapsedMs - phaseStartMs;
                const AnimationConfig& animCfg = cachedAnimationConfig(ps.pieceCode, ps.animState);
                ps.frameIndex = computeAnimationFrameIndex(phaseMs, animCfg.framesPerSec, animCfg.frameCount, animCfg.isLoop);

                snap.pieces.push_back(ps);
            }
        }

        if (state.selection.active) {
            snap.selectedCell = state.selection.cell;
        }

        return snap;
    }

}
