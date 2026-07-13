#include "doctest.h"

#include "../input/Controller.hpp"
#include "../engine/GameEngine.hpp"
#include "../texttests/ScriptRunner.hpp"
#include "../realtime/RealTimeArbiter.hpp"
#include "../model/Board.hpp"
#include "../model/GameState.hpp"
#include "../model/Piece.hpp"
#include "../io/BoardParser.hpp"
#include "../io/BoardPrinter.hpp"

#include <sstream>
#include <iostream>

namespace {
    GameState makeState(const std::vector<std::string>& boardLines) {
        GameState st;
        st.board = parseBoard(boardLines);
        return st;
    }

    std::string tokenAt(const Board& b, Position pos) {
        auto piece = b.pieceAt(pos);
        return piece ? tokenFromPiece(*piece) : EMPTY_TOKEN;
    }
}

TEST_CASE("advanceTime keeps a move active before its arrival time") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 1000; m.piece = "wR";
    st.arbiter.startMotion(m);
    st.elapsedMs = 500;

    st.arbiter.advanceTime(st.elapsedMs, st.board);

    REQUIRE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == EMPTY_TOKEN);
}

TEST_CASE("advanceTime lands a move onto an empty target once due") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 1000; m.piece = "wR";
    st.arbiter.startMotion(m);
    st.elapsedMs = 1000;

    st.arbiter.advanceTime(st.elapsedMs, st.board);

    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("advanceTime captures an enemy occupying the target") {
    GameState st = makeState({"wR . . bP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);
    st.elapsedMs = 500;

    st.arbiter.advanceTime(st.elapsedMs, st.board);

    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("advanceTime leaves a piece at its source when the target turned friendly") {
    // With the mid-flight-disappearance bug fixed, the source piece is never
    // removed in the first place, so there is nothing to "restore" - it
    // simply never left. The board fixture reflects that reality (the wR is
    // actually still sitting at its source).
    GameState st = makeState({"wR . . wP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);
    st.elapsedMs = 500;

    st.arbiter.advanceTime(st.elapsedMs, st.board);

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK(tokenAt(st.board, {0, 3}) == "wP");
}

TEST_CASE("advanceTime loses a piece when the target turned friendly and the source is occupied") {
    GameState st = makeState({"wN . . wP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);
    st.elapsedMs = 500;

    st.arbiter.advanceTime(st.elapsedMs, st.board);

    CHECK(tokenAt(st.board, {0, 0}) == "wN");
    CHECK(tokenAt(st.board, {0, 3}) == "wP");
}

// NOTE: the old "resolveMoves settles multiple due moves in arrival order"
// test was deleted here. It relied on constructing a GameState with TWO
// simultaneous PieceMove entries to exercise the old due/sort logic. That
// scenario is now structurally impossible to build - RealTimeArbiter holds
// at most one PieceMove (std::optional<PieceMove>), and startMotion() throws
// if called while a motion is already active. There is no way to mechanically
// translate this test; it tested behavior that can no longer exist.

TEST_CASE("sendMove keeps the piece at its source and queues an active motion for a legal move") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3);

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.selection.active);

    // Confirm the queued motion is indeed the rook heading to (0,3): advance
    // past its travel time (1.0 cells/sec, 3 cells -> 3000ms) and check it
    // actually arrives there.
    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("sendMove ignores an illegal move and leaves the board untouched") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 1, 1); // diagonal - illegal for a rook

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("sendMove computes duration from piece speed and travel distance") {
    GameState st = makeState({"wQ . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3); // queen: 4 cells/sec, 3 cells travelled -> 750ms

    REQUIRE(st.arbiter.hasActiveMotion());

    // Not yet arrived just before the computed duration...
    st.arbiter.advanceTime(749, st.board);
    CHECK(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == EMPTY_TOKEN);

    // ...but arrived exactly at 750ms, confirming the computed duration.
    st.arbiter.advanceTime(750, st.board);
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == "wQ");
}

TEST_CASE("sendMove rejects a second move request while any motion is active, even for a different piece") {
    // Global "one active motion at a time" rule: while piece A is mid-flight,
    // NO new move may be accepted - not even for a completely different,
    // otherwise-legal move by piece B.
    GameState st = makeState({"wR . . .", ". . . wN", ". . . .", ". . . ."});

    // A: select the rook at (0,0) and send it toward (0,3).
    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 3);
    REQUIRE(st.arbiter.hasActiveMotion());

    // B: attempt to select and move the knight at (1,3) to (3,2) - a legal
    // knight move on an empty square - before A arrives.
    st.selection = {true, {1, 3}, 0};
    sendMove(st, 3, 2);

    // The second request must be rejected purely because a motion is active:
    // B never moves, and its selection is reset like any rejected move.
    CHECK(tokenAt(st.board, {1, 3}) == "wN");
    CHECK_FALSE(st.selection.active);

    // A's original motion is completely unaffected by B's rejected attempt:
    // advancing to its arrival still lands the rook at (0,3) exactly as
    // queued, and the knight never moved at all.
    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
    CHECK(tokenAt(st.board, {1, 3}) == "wN");
}

TEST_CASE("sendMove accepts a new move immediately after the previous motion completes, with no cooldown") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});

    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 3); // rook: 3 cells at 1.0 cells/sec -> 3000ms
    REQUIRE(st.arbiter.hasActiveMotion());

    handleWait(st, 3000); // arrives exactly on time
    REQUIRE_FALSE(st.arbiter.hasActiveMotion());
    REQUIRE(tokenAt(st.board, {0, 3}) == "wR");

    // Immediately (zero additional wait) request a new move - nothing should
    // block it now that the previous motion has fully completed.
    st.selection = {true, {0, 3}, st.elapsedMs};
    sendMove(st, 0, 0); // rook travels back, same 3-cell distance -> 3000ms

    CHECK(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == "wR"); // still at source until this motion arrives

    handleWait(st, 3000);
    CHECK(tokenAt(st.board, {0, 0}) == "wR");
}

TEST_CASE("handleClick opens a fresh selection when clicking an idle piece") {
    GameState st = makeState({"wK . . .", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5); // inside cell (0,0)

    CHECK(st.selection.active);
    CHECK(st.selection.cell.row == 0);
    CHECK(st.selection.cell.col == 0);
}

TEST_CASE("handleClick reselects when clicking another piece of the same color") {
    GameState st = makeState({"wK . wQ .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 205, 5); // cell (0,2), also white

    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 2);
}

TEST_CASE("handleClick completes a pending selection when clicking an empty cell") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 305, 5); // empty cell (0,3)

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.arbiter.hasActiveMotion());

    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("handleClick completes a pending selection as a capture on an enemy cell") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 305, 5); // the black pawn at (0,3)

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.selection.active);

    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR"); // captured the black pawn
}

TEST_CASE("handleClick with no pending selection opens a selection regardless of piece color") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 305, 5); // black's pawn, nobody is pending

    CHECK(tokenAt(st.board, {0, 3}) == "bP");
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 3);
}

TEST_CASE("handleClick ignores clicks outside the board") {
    GameState st = makeState({"wK ."});
    Controller::click(st, -5, -5);
    Controller::click(st, 10000, 10000);

    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleClick cancels an active selection on an outside-board click") {
    GameState st = makeState({"wR . . bN", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5); // select the rook at (0,0)
    REQUIRE(st.selection.active);

    Controller::click(st, -5, -5); // outside the board -> must cancel the selection

    CHECK_FALSE(st.selection.active);

    // The next in-bounds click must open a fresh selection on the enemy
    // knight, not be misread as completing a move from the stale selection.
    Controller::click(st, 305, 5); // click on the black knight at (0,3)

    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");   // rook never moved
    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 3);          // freshly selected the knight, not a completed move
}

TEST_CASE("handleClick on an empty cell with no pending selection is a no-op") {
    GameState st = makeState({". . .", ". . .", ". . ."});
    Controller::click(st, 5, 5);
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleWait advances the clock and resolves due moves") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 100; m.piece = "wR";
    st.arbiter.startMotion(m);

    handleWait(st, 150);

    CHECK(st.elapsedMs == 150);
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("runCommands executes click, wait and print in sequence") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "click 5 5",
        "click 305 5",
        "wait 3000", // rook: 1.0 cells/sec, 3 cells travelled -> 3000ms to arrive
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}
