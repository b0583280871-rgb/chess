#include "doctest.h"

#include "../realtime/RealTimeArbiter.hpp"
#include "../model/Board.hpp"
#include "../model/GameState.hpp"
#include "../model/Piece.hpp"
#include "../io/BoardParser.hpp"

namespace {
    GameState makeState(const std::vector<std::string>& boardLines) {
        GameState st;
        st.board = parseBoard(boardLines);
        return st;
    }
}

TEST_CASE("activeMotionForPiece returns nullopt when no motion is active") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});

    CHECK_FALSE(st.arbiter.activeMotionForPiece({0, 0}).has_value());
}

TEST_CASE("activeMotionForPiece returns the active motion for its source cell") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 2}; m.startMs = 0; m.durationMs = 2000; m.piece = "wR";
    st.arbiter.startMotion(m);

    std::optional<PieceMove> found = st.arbiter.activeMotionForPiece({0, 0});

    REQUIRE(found.has_value());
    CHECK(found->from == m.from);
    CHECK(found->to == m.to);
    CHECK(found->startMs == m.startMs);
    CHECK(found->durationMs == m.durationMs);
    CHECK(found->piece == m.piece);
}

TEST_CASE("activeMotionForPiece returns nullopt for a cell that is not the active motion's source") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 2}; m.startMs = 0; m.durationMs = 2000; m.piece = "wR";
    st.arbiter.startMotion(m);

    CHECK_FALSE(st.arbiter.activeMotionForPiece({0, 2}).has_value());   // the destination, not the source
    CHECK_FALSE(st.arbiter.activeMotionForPiece({1, 1}).has_value());   // unrelated cell
}

TEST_CASE("activeMotionForPiece still returns the motion mid-progress, and nullopt once it has arrived") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 2}; m.startMs = 0; m.durationMs = 2000; m.piece = "wR";
    st.arbiter.startMotion(m);

    CHECK(st.arbiter.activeMotionForPiece({0, 0}).has_value());   // not due yet, still tracked

    st.arbiter.advanceTime(1000, st.board);   // halfway - still not due
    CHECK(st.arbiter.activeMotionForPiece({0, 0}).has_value());

    st.arbiter.advanceTime(2000, st.board);   // arrival - motion resolves and clears
    CHECK_FALSE(st.arbiter.activeMotionForPiece({0, 0}).has_value());
}
