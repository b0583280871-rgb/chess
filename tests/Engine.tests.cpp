#include "doctest.h"

#include "input/Controller.hpp"
#include "engine/GameEngine.hpp"
#include "texttests/ScriptRunner.hpp"
#include "realtime/RealTimeArbiter.hpp"
#include "model/Board.hpp"
#include "model/GameState.hpp"
#include "model/Piece.hpp"
#include "io/BoardParser.hpp"
#include "io/BoardPrinter.hpp"

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

TEST_CASE("resolveMoves keeps a move active before its arrival time") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 1000; m.piece = "wR";
    st.activeMoves.push_back(m);
    st.elapsedMs = 500;

    resolveMoves(st);

    REQUIRE(st.activeMoves.size() == 1);
    CHECK(tokenAt(st.board, {0, 3}) == EMPTY_TOKEN);
}

TEST_CASE("resolveMoves lands a move onto an empty target once due") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 1000; m.piece = "wR";
    st.activeMoves.push_back(m);
    st.elapsedMs = 1000;

    resolveMoves(st);

    CHECK(st.activeMoves.empty());
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("resolveMoves captures an enemy occupying the target") {
    GameState st = makeState({"wR . . bP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.activeMoves.push_back(m);
    st.elapsedMs = 500;

    resolveMoves(st);

    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("resolveMoves leaves a piece at its source when the target turned friendly") {
    // Corrected version of the old "bounces a piece home" test: with the
    // mid-flight-disappearance bug fixed, the source piece was never removed
    // in the first place, so there is nothing to "restore" - it simply
    // never left. The board fixture below reflects that reality (the wR is
    // actually still sitting at its source), instead of the old fixture that
    // simulated the bug by starting with an empty source.
    GameState st = makeState({"wR . . wP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.activeMoves.push_back(m);
    st.elapsedMs = 500;

    resolveMoves(st);

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK(tokenAt(st.board, {0, 3}) == "wP");
}

TEST_CASE("resolveMoves loses a piece when the target turned friendly and the source is occupied") {
    GameState st = makeState({"wN . . wP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.activeMoves.push_back(m);
    st.elapsedMs = 500;

    resolveMoves(st);

    CHECK(tokenAt(st.board, {0, 0}) == "wN");
    CHECK(tokenAt(st.board, {0, 3}) == "wP");
}

TEST_CASE("resolveMoves settles multiple due moves in arrival order") {
    // Board fixture updated to actually contain the pieces at their move
    // sources (wP at (0,0), bP at (0,2)) - resolveMoves now looks up real
    // pieces via pieceAt instead of writing PieceMove.piece strings blindly,
    // so a source with nothing there is simply skipped.
    GameState st = makeState({"wP . bP"});
    PieceMove first; first.from = {0, 0}; first.to = {0, 1};
    first.startMs = 0; first.durationMs = 200; first.piece = "wP";
    PieceMove second; second.from = {0, 2}; second.to = {0, 1};
    second.startMs = 0; second.durationMs = 100; second.piece = "bP";
    st.activeMoves = {first, second};
    st.elapsedMs = 300;

    resolveMoves(st);

    CHECK(tokenAt(st.board, {0, 1}) == "wP");
}

TEST_CASE("sendMove keeps the piece at its source and queues an active move for a legal move") {
    // Renamed from "clears the source": with the mid-flight bug fixed,
    // sendMove no longer clears the source cell - the piece stays there
    // until RealTimeArbiter confirms arrival.
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3);

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.activeMoves.size() == 1);
    CHECK(st.activeMoves[0].piece == "wR");
    CHECK(st.activeMoves[0].to.col == 3);
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("sendMove ignores an illegal move and leaves the board untouched") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 1, 1); // diagonal - illegal for a rook

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK(st.activeMoves.empty());
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("sendMove computes duration from piece speed and travel distance") {
    GameState st = makeState({"wQ . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3); // queen: 4 cells/sec, 3 cells travelled

    REQUIRE(st.activeMoves.size() == 1);
    CHECK(st.activeMoves[0].durationMs == 750);
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
    REQUIRE(st.activeMoves.size() == 1);
    CHECK(st.activeMoves[0].to.col == 3);
}

TEST_CASE("handleClick completes a pending selection as a capture on an enemy cell") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 305, 5); // the black pawn at (0,3)

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.activeMoves.size() == 1);
    CHECK(st.activeMoves[0].to.col == 3);
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleClick with no pending selection opens a selection regardless of piece color") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 305, 5); // black's pawn, nobody is pending

    CHECK(tokenAt(st.board, {0, 3}) == "bP");
    CHECK(st.activeMoves.empty());
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

    CHECK(st.activeMoves.empty());
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
    st.activeMoves.push_back(m);

    handleWait(st, 150);

    CHECK(st.elapsedMs == 150);
    CHECK(st.activeMoves.empty());
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
