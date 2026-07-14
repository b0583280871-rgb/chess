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

    GameState state;

    try {
        Sections sections = parseSections(input);
        state.board = parseBoard(sections.boardLines);

        ScriptRunner::run(sections.commandLines, state);
    } catch (const BoardError& e) {
        std::cout << "ERROR " << e.code() << '\n';
        return 0;
    }

    return 0;
}
