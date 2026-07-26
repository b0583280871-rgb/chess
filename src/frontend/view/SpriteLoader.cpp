#include "SpriteLoader.hpp"

#include <unordered_map>

#include "../../backend/rules/config.hpp"

namespace SpriteLoader
{

    Img &getCachedPieceSprite(const std::string &pieceCode, const std::string &state)
    {
        static std::unordered_map<std::string, Img> cache;

        const std::string key = pieceCode + "_" + state;
        auto it = cache.find(key);
        if (it != cache.end())
            return it->second;

        Img img;
        img.read(PIECES_ROOT + pieceCode + "/states/" + state + "/sprites/1.png",
                 {config::CELL_SIZE, config::CELL_SIZE}, true);
        return cache.emplace(key, std::move(img)).first->second;
    }

    Img &getCachedPieceFrame(const std::string &pieceCode, const std::string &state, int frameIndex)
    {
        static std::unordered_map<std::string, Img> cache;

        const std::string key = pieceCode + "_" + state + "_" + std::to_string(frameIndex);
        auto it = cache.find(key);
        if (it != cache.end())
            return it->second;

        Img img;
        img.read(PIECES_ROOT + pieceCode + "/states/" + state + "/sprites/" + std::to_string(frameIndex) + ".png",
                 {config::CELL_SIZE, config::CELL_SIZE}, true);
        return cache.emplace(key, std::move(img)).first->second;
    }

}
