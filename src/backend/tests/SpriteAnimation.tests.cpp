#include "doctest.h"

#include "../engine/GameEngine.hpp"

#include "../io/BoardParser.hpp"

// --- Pure frame-index math, synthetic config values, no file I/O ----------

TEST_CASE("computeAnimationFrameIndex starts at frame 1 for a looping animation") {
    // framesPerSec=9, frameCount=5, isLoop=true (matches the real BB idle config)
    CHECK(GameEngine::computeAnimationFrameIndex(0, 9, 5, true) == 1);
}

TEST_CASE("computeAnimationFrameIndex advances and wraps for a looping animation") {
    CHECK(GameEngine::computeAnimationFrameIndex(500, 9, 5, true) == 5);   // rawFrame=4 -> 1+(4%5)
    CHECK(GameEngine::computeAnimationFrameIndex(1000, 9, 5, true) == 5);  // rawFrame=9 -> 1+(9%5)
}

TEST_CASE("computeAnimationFrameIndex freezes on the last frame for a non-looping animation") {
    // framesPerSec=10, frameCount=5, isLoop=false (matches the real BB jump config)
    CHECK(GameEngine::computeAnimationFrameIndex(0, 10, 5, false) == 1);
    CHECK(GameEngine::computeAnimationFrameIndex(200, 10, 5, false) == 3);    // rawFrame=2 -> 1+2
    CHECK(GameEngine::computeAnimationFrameIndex(10000, 10, 5, false) == 5);  // way past the end, frozen
}

TEST_CASE("computeAnimationFrameIndex clamps a negative phase time to the first frame") {
    CHECK(GameEngine::computeAnimationFrameIndex(-500, 9, 5, true) == 1);
}

// --- Full pipeline through GameEngine::snapshot, real config.json/sprites -
// (BB = black bishop; idle/move configs are consistent across all piece
// codes in assets/pieces_classic/: idle is 9 fps/5 frames, move is 10 fps/5
// frames, both looping.)

TEST_CASE("GameEngine::snapshot computes frameIndex 1 at the start of a looping idle animation") {
    GameState st;
    st.board = parseBoard({"bB . .", ". . .", ". . ."});
    st.elapsedMs = 0;

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.pieces.size() == 1);
    CHECK(snap.pieces[0].animState == "idle");
    CHECK(snap.pieces[0].frameIndex == 1);
}

TEST_CASE("GameEngine::snapshot advances frameIndex over simulated time for a looping idle animation") {
    GameState st;
    st.board = parseBoard({"bB . .", ". . .", ". . ."});
    st.elapsedMs = 500;

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.pieces.size() == 1);
    CHECK(snap.pieces[0].frameIndex == 5);   // idle: 9 fps, 5 frames -> rawFrame=4 -> 1+(4%5)
}

TEST_CASE("GameEngine::snapshot bases a moving piece's frameIndex on the active motion's startMs") {
    GameState st;
    st.board = parseBoard({"bB . .", ". . .", ". . ."});

    PieceMove m;
    m.from = {0, 0};
    m.to = {0, 1};
    m.startMs = 1000;
    m.durationMs = 2000;
    m.piece = "bB";
    st.arbiter.startMotion(m);

    st.elapsedMs = 1300;   // 300ms into the motion

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.pieces.size() == 1);
    CHECK(snap.pieces[0].animState == "move");
    CHECK(snap.pieces[0].frameIndex == 4);   // move: 10 fps, 5 frames -> rawFrame=3 -> 1+(3%5)
}
