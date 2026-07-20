#include "../backend/engine/GameEngine.hpp"
#include "../backend/input/Controller.hpp"
#include "view/Renderer.hpp"
#include "../backend/io/BoardParser.hpp"
#include "../backend/io/BoardFormat.hpp"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

namespace {
    const std::string WINDOW_NAME = "Kung Fu Chess";

    void onMouse(int event, int x, int y, int /*flags*/, void* userdata) {
        if (event != cv::EVENT_LBUTTONDOWN) return;

        GameState* state = static_cast<GameState*>(userdata);
        // live_demo is a local single-process demo with no real player
        // identity/turn concept - Color::White is a fixed, agreed default
        // (matches the convention elsewhere of the first player being white).
        Controller::click(*state, x, y, Color::White);
    }
}

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

        GameState state;
        state.board = parseBoard(sections.boardLines);

        bool gameOverKeySeen = false;

        cv::namedWindow(WINDOW_NAME);
        cv::setMouseCallback(WINDOW_NAME, onMouse, static_cast<void*>(&state));

        auto previousTime = std::chrono::steady_clock::now();

        while (true) {
            auto now = std::chrono::steady_clock::now();
            long deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime).count();
            previousTime = now;

            handleWait(state, deltaMs);

            GameSnapshot snapshot = GameEngine::snapshot(state);
            Img canvas = renderFrame(snapshot);

            cv::imshow(WINDOW_NAME, canvas.get_mat());
            int key = cv::waitKey(1);

            if (key == 27 || key == 'q' || key == 'Q') {  
                break;
            }

            if (state.gameOver) {
                if (gameOverKeySeen) break;
                if (key != -1) gameOverKeySeen = true;
            }
        }

        cv::destroyAllWindows();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "live_demo failed: " << e.what() << std::endl;
        return 1;
    }
}
