#include "Engine.hpp"

#include <iostream>
#include <sstream>

#include "engine/GameEngine.hpp"
#include "input/Controller.hpp"
#include "io/BoardParser.hpp"
#include "io/BoardPrinter.hpp"

void handleClick(GameState& st, int x, int y) {
    Controller::click(st, x, y);
}

void runCommands(const std::vector<std::string>& commands, GameState& st) {
    for (const auto& command : commands) {
        std::istringstream ss(command);
        std::string verb;
        ss >> verb;

        if (verb == "click") {
            int x, y;
            ss >> x >> y;
            handleClick(st, x, y);
        } else if (verb == "wait") {
            long ms; ss >> ms;
            handleWait(st, ms);
        } else if (verb == "print") {
            std::string rest; std::getline(ss, rest);
            if (trim(rest) == "board") std::cout << formatBoard(st.board);
        }
    }
}
