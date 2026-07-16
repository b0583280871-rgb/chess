#include "view/GameSnapshot.hpp"
#include "view/Renderer.hpp"
#include "../backend/io/BoardParser.hpp"
#include "../backend/io/BoardFormat.hpp"

#include <iostream>

int main() {
    try {
        const std::string boardText =
            io_format::SECTION_BOARD + "\n"
            "bR bN bB bQ bK bB bN bR\n"
            "bP bP bP bP bP bP bP bP\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            "wP wP wP wP wP wP wP wP\n"
            "wR wN wB wQ wK wB wN wR\n" +
            io_format::SECTION_COMMANDS + "\n";

        Sections sections = parseSections(boardText);

        GameState st;
        st.board = parseBoard(sections.boardLines);

        GameSnapshot snapshot = buildSnapshot(st);
        Img frame = renderFrame(snapshot);
        frame.show();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to render frame: " << e.what() << std::endl;
        return 1;
    }
}
