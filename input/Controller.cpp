#include "input/Controller.hpp"

#include "model/Board.hpp"
#include "input/BoardMapper.hpp"
#include "engine/GameEngine.hpp"

namespace Controller {

    void click(GameState& st, int x, int y) {
        auto cell = pixelToCell(x, y, st.board);
        if (!cell) return;

        int row = cell->first;
        int col = cell->second;

        const std::string& token = st.board.grid[row][col];

        if (st.selection.active) {
            const std::string& selectedTok = st.board.grid[st.selection.row][st.selection.col];
            bool sameSide = !isEmpty(token) && colorOf(token) == colorOf(selectedTok);
            if (sameSide) {
                st.selection = {true, row, col, st.elapsedMs};   // reselect
            } else {
                sendMove(st, row, col);                           // complete: move or capture
            }
            return;
        }

        if (!isEmpty(token)) {
            st.selection = {true, row, col, st.elapsedMs};       // open a fresh selection
        }
    }

}
