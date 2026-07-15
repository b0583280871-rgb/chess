#include "view/SpriteLoader.hpp"
#include <iostream>

int main() {
    try {
        Img img = SpriteLoader::loadPieceIdleSprite("BB");
        img.show();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load sprite: " << e.what() << std::endl;
        return 1;
    }
}
