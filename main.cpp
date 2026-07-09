#include <iostream>
#include <string>

#include "model/Board.hpp"
#include "texttests/ScriptRunner.hpp"
#include "model/GameState.hpp"
#include "rules/config.hpp"
#include "io/BoardParser.hpp"

int main() {
    std::string input, line;
    while (std::getline(std::cin, line)) input += line + '\n';

    Sections sections = parseSections(input);

    GameState state;
    state.board = parseBoard(sections.boardLines);

    try {
        validateBoard(state.board);
    } catch (const BoardError& e) {
        std::cout << "ERROR " << e.code() << '\n';
        return 0;
    }

    ScriptRunner::run(sections.commandLines, state);
    return 0;
}
