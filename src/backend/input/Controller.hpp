#pragma once

#include "../model/GameState.hpp"
#include "../model/Piece.hpp"

namespace Controller {

    void click(GameState& st, int x, int y, Color playerColor);

    void jump(GameState& st, int x, int y);

}
