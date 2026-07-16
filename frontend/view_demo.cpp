#include "view/GameSnapshot.hpp"
#include "view/Renderer.hpp"
#include "../backend/io/BoardParser.hpp"

#include <iostream>

int main() {
    try {
        GameState st;
        st.board = parseBoard({
            "bR bN bB bQ bK bB bN bR",
            "bP bP bP bP bP bP bP bP",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            "wP wP wP wP wP wP wP wP",
            "wR wN wB wQ wK wB wN wR"
        });

        GameSnapshot snapshot = buildSnapshot(st);
        Img frame = renderFrame(snapshot);
        frame.show();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to render frame: " << e.what() << std::endl;
        return 1;
    }
}
