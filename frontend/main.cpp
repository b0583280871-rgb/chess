#include <iostream>
#include <string>

#include "model/Board.hpp"
#include "texttests/ScriptRunner.hpp"
#include "model/GameState.hpp"
#include "rules/config.hpp"
#include "io/BoardParser.hpp"

#include <iostream>
#include <string>

#include "img.hpp"

int main() {
    try {
        std::cout << "Opening OpenCV window..." << std::endl;
        Img img;
        img.create({640, 480}, {255, 255, 255});
        img.put_text("Hello, Img!", 150, 360, 1.0, {0, 0, 0}, 2);
        img.show();
        std::cout << "Window closed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}