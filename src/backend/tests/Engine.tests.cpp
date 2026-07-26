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

TEST_CASE("advanceTime's ArrivalEvent reports a captured piece with Captured state") {
    GameState st = makeState({"wR . . bP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(500, st.board);

    CHECK(event.pieceArrived);
    REQUIRE(event.capturedPiece.has_value());
    CHECK(event.capturedPiece->kind == Kind::Pawn);
    CHECK(event.capturedPiece->color == Color::Black);
    CHECK(event.capturedPiece->state == PieceState::Captured);
}

TEST_CASE("advanceTime's ArrivalEvent has no captured piece when landing on an empty cell") {
    GameState st = makeState({"wR . . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(500, st.board);

    CHECK(event.pieceArrived);
    CHECK_FALSE(event.capturedPiece.has_value());
}

TEST_CASE("advanceTime's ArrivalEvent reports pieceArrived false when the target turned friendly") {
    GameState st = makeState({"wR . . wP"});
    PieceMove m; m.from = {0, 0}; m.to = {0, 3};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(500, st.board);

    CHECK_FALSE(event.pieceArrived);
    CHECK_FALSE(event.capturedPiece.has_value());
}

TEST_CASE("advanceTime promotes a white pawn to a queen when it reaches the last row") {
    GameState st = makeState({". . .", ". . .", "wP . ."});
    PieceMove m; m.from = {2, 0}; m.to = {0, 0};
    m.startMs = 0; m.durationMs = 500; m.piece = "wP";
    st.arbiter.startMotion(m);

    st.arbiter.advanceTime(500, st.board);

    CHECK(tokenAt(st.board, {0, 0}) == "wQ");
}

TEST_CASE("advanceTime promotes a black pawn reaching the last row for its color") {
    GameState st = makeState({"bP . .", ". . .", ". . ."});
    PieceMove m; m.from = {0, 0}; m.to = {2, 0};
    m.startMs = 0; m.durationMs = 500; m.piece = "bP";
    st.arbiter.startMotion(m);

    st.arbiter.advanceTime(500, st.board);

    CHECK(tokenAt(st.board, {2, 0}) == "bQ");
}

TEST_CASE("advanceTime leaves a pawn as a pawn when it doesn't reach the last row") {
    GameState st = makeState({". . .", ". . .", "wP . ."});
    PieceMove m; m.from = {2, 0}; m.to = {1, 0};
    m.startMs = 0; m.durationMs = 500; m.piece = "wP";
    st.arbiter.startMotion(m);

    st.arbiter.advanceTime(500, st.board);

    CHECK(tokenAt(st.board, {1, 0}) == "wP");
}

TEST_CASE("advanceTime does not promote a non-pawn piece that reaches what would be a pawn's last row") {
    GameState st = makeState({". . .", ". . .", "wR . ."});
    PieceMove m; m.from = {2, 0}; m.to = {0, 0};
    m.startMs = 0; m.durationMs = 500; m.piece = "wR";
    st.arbiter.startMotion(m);

    st.arbiter.advanceTime(500, st.board);

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
}

TEST_CASE("startJump activates a jump for an idle piece") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    CHECK_FALSE(st.arbiter.hasActiveJump());

    st.arbiter.startJump({0, 0}, 0);

    CHECK(st.arbiter.hasActiveJump());
    CHECK(st.arbiter.isPieceCurrentlyJumping({0, 0}));
}

TEST_CASE("startJump throws if a jump is already active") {
    GameState st = makeState({"wR . wN", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);

    CHECK_THROWS_AS(st.arbiter.startJump({0, 2}, 0), RealTimeArbiterError);
}

TEST_CASE("startJump throws for a piece that is currently the active move's source") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    PieceMove m; m.from = {0, 0}; m.to = {0, 2}; m.startMs = 0; m.durationMs = 1000; m.piece = "wR";
    st.arbiter.startMotion(m);

    CHECK_THROWS_AS(st.arbiter.startJump({0, 0}, 0), RealTimeArbiterError);
}

TEST_CASE("advanceTime leaves an active jump untouched before its 1000ms window elapses") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);

    st.arbiter.advanceTime(999, st.board);

    CHECK(st.arbiter.hasActiveJump());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");
}

TEST_CASE("advanceTime lands a jump with zero board change once its window elapses") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);

    st.arbiter.advanceTime(1000, st.board);

    CHECK_FALSE(st.arbiter.hasActiveJump());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");
}

TEST_CASE("advanceTime intercepts an enemy move arriving at an active jump's cell") {
    GameState st = makeState({"wR . bN", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);   // wR jumps in place

    PieceMove m; m.from = {0, 2}; m.to = {0, 0}; m.startMs = 0; m.durationMs = 999; m.piece = "bN";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(999, st.board);

    CHECK_FALSE(st.arbiter.hasActiveJump());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");          // jumper unchanged, still present
    CHECK(tokenAt(st.board, {0, 2}) == EMPTY_TOKEN);   // arriving piece never lands anywhere
    REQUIRE(event.capturedPiece.has_value());
    CHECK(event.capturedPiece->kind == Kind::Knight);
    CHECK(event.capturedPiece->color == Color::Black);
    CHECK(event.capturedPiece->state == PieceState::Captured);
}

TEST_CASE("advanceTime treats a friendly arrival at a jump's cell as an ordinary friendly-block, not interception") {
    GameState st = makeState({"wR . wN", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);   // wR jumps in place

    PieceMove m; m.from = {0, 2}; m.to = {0, 0}; m.startMs = 0; m.durationMs = 999; m.piece = "wN";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(999, st.board);

    CHECK(st.arbiter.hasActiveJump());                 // jump unaffected, still active
    CHECK(tokenAt(st.board, {0, 0}) == "wR");           // jumper still there
    CHECK(tokenAt(st.board, {0, 2}) == "wN");           // friendly move lost, arriving piece stays put
    CHECK_FALSE(event.pieceArrived);
    CHECK_FALSE(event.capturedPiece.has_value());
}

TEST_CASE("advanceTime resolves an ordinary capture when a move arrives after the jump already expired") {
    GameState st = makeState({"wR . bN", ". . .", ". . ."});
    st.arbiter.startJump({0, 0}, 0);
    st.arbiter.advanceTime(1000, st.board);            // jump expires independently first
    REQUIRE_FALSE(st.arbiter.hasActiveJump());

    PieceMove m; m.from = {0, 2}; m.to = {0, 0}; m.startMs = 1000; m.durationMs = 500; m.piece = "bN";
    st.arbiter.startMotion(m);

    ArrivalEvent event = st.arbiter.advanceTime(1500, st.board);

    CHECK(tokenAt(st.board, {0, 0}) == "bN");          // ordinary capture-by-landing, not interception
    REQUIRE(event.capturedPiece.has_value());
    CHECK(event.capturedPiece->kind == Kind::Rook);
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

    sendMove(st, 0, 3); // queen: 1.0 cells/sec, 3 cells travelled -> 3000ms

    REQUIRE(st.arbiter.hasActiveMotion());

    // Not yet arrived just before the computed duration...
    st.arbiter.advanceTime(2999, st.board);
    CHECK(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 3}) == EMPTY_TOKEN);

    // ...but arrived exactly at 3000ms, confirming the computed duration.
    st.arbiter.advanceTime(3000, st.board);
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

TEST_CASE("handleWait sets gameOver true when a king is captured on arrival") {
    GameState st = makeState({"wR . . bK", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3); // rook captures the black king
    REQUIRE(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.gameOver);

    handleWait(st, 3000); // 3 cells at 1.0 cells/sec -> 3000ms

    CHECK(st.gameOver);
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("sendMove after game over leaves the board completely unchanged") {
    GameState st = makeState({"wR . . bK", ". . . wN", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 3);
    handleWait(st, 3000);
    REQUIRE(st.gameOver);

    const std::string knightBefore = tokenAt(st.board, {1, 3});
    const std::string rookBefore   = tokenAt(st.board, {0, 3});

    st.selection = {true, {1, 3}, st.elapsedMs};
    sendMove(st, 3, 2); // otherwise-legal knight move, attempted after game over

    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {1, 3}) == knightBefore);
    CHECK(tokenAt(st.board, {0, 3}) == rookBefore);
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("capturing a non-king piece does not end the game") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    sendMove(st, 0, 3); // rook captures the black pawn
    handleWait(st, 3000);

    CHECK_FALSE(st.gameOver);
    CHECK(tokenAt(st.board, {0, 3}) == "wR"); // capture went through normally
}

TEST_CASE("startJump succeeds for an idle, non-moving, non-jumping piece") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});

    startJump(st, {0, 0});

    CHECK(st.arbiter.hasActiveJump());
    CHECK(st.arbiter.isPieceCurrentlyJumping({0, 0}));
}

TEST_CASE("startJump is a silent no-op for a piece that is currently the active move's source") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 2);
    REQUIRE(st.arbiter.hasActiveMotion());

    startJump(st, {0, 0});

    CHECK_FALSE(st.arbiter.hasActiveJump());
}

TEST_CASE("startJump is a silent no-op while a jump is already active anywhere on the board") {
    GameState st = makeState({"wR . wN", ". . .", ". . ."});
    startJump(st, {0, 0});
    REQUIRE(st.arbiter.hasActiveJump());

    startJump(st, {0, 2});

    CHECK(st.arbiter.isPieceCurrentlyJumping({0, 0}));
    CHECK_FALSE(st.arbiter.isPieceCurrentlyJumping({0, 2}));
}

TEST_CASE("sendMove is rejected for a piece that is currently jumping") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});
    startJump(st, {0, 0});
    REQUIRE(st.arbiter.hasActiveJump());

    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 2);

    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK_FALSE(st.selection.active);
}

TEST_CASE("sendMove and startJump both no-op once the game is over") {
    GameState st = makeState({"wR . . bK", ". . . wN", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};
    sendMove(st, 0, 3);      // rook captures the black king
    handleWait(st, 3000);
    REQUIRE(st.gameOver);

    st.selection = {true, {1, 3}, st.elapsedMs};
    sendMove(st, 3, 2);      // otherwise-legal knight move
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {1, 3}) == "wN");

    startJump(st, {1, 3});
    CHECK_FALSE(st.arbiter.hasActiveJump());
}

TEST_CASE("a king destroyed via jump interception sets gameOver true") {
    GameState st = makeState({"wR bK .", ". . .", ". . ."});

    startJump(st, {0, 0});    // wR jumps in place

    st.selection = {true, {0, 1}, 0};
    sendMove(st, 0, 0);       // bK attempts to move onto wR's cell

    REQUIRE(st.arbiter.hasActiveMotion());
    REQUIRE(st.arbiter.hasActiveJump());

    handleWait(st, 1000);     // the king's move and the jump's window elapse at the same tick

    CHECK(st.gameOver);
    CHECK(tokenAt(st.board, {0, 0}) == "wR");    // jumper survives, unmoved
    CHECK(tokenAt(st.board, {0, 1}) == EMPTY_TOKEN);
    CHECK_FALSE(st.arbiter.hasActiveJump());     // jump concluded via interception
}

TEST_CASE("handleClick opens a fresh selection when clicking an idle piece") {
    GameState st = makeState({"wK . . .", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::White); // inside cell (0,0)

    CHECK(st.selection.active);
    CHECK(st.selection.cell.row == 0);
    CHECK(st.selection.cell.col == 0);
}

TEST_CASE("handleClick reselects when clicking another piece of the same color") {
    GameState st = makeState({"wK . wQ .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 205, 5, Color::White); // cell (0,2), also white

    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 2);
}

TEST_CASE("handleClick completes a pending selection when clicking an empty cell") {
    GameState st = makeState({"wR . . .", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 305, 5, Color::White); // empty cell (0,3)

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.arbiter.hasActiveMotion());

    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR");
}

TEST_CASE("handleClick completes a pending selection as a capture on an enemy cell") {
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});
    st.selection = {true, {0, 0}, 0};

    Controller::click(st, 305, 5, Color::White); // the black pawn at (0,3)

    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    REQUIRE(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.selection.active);

    st.arbiter.advanceTime(3000, st.board);
    CHECK(tokenAt(st.board, {0, 3}) == "wR"); // captured the black pawn
}

TEST_CASE("handleClick with no pending selection ignores a click on an opposing piece") {
    // Was "...opens a selection regardless of piece color" before color
    // enforcement (S5.7) - that permissive behavior is exactly what this
    // step removes, so the test now demonstrates the opposite.
    GameState st = makeState({"wR . . bP", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 305, 5, Color::White); // black's pawn, but a white player is clicking

    CHECK(tokenAt(st.board, {0, 3}) == "bP");      // piece untouched
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK_FALSE(st.selection.active);              // no selection opened - wrong color
}

TEST_CASE("handleClick does not let a white player select a black piece") {
    GameState st = makeState({"bK . . .", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::White); // black king at (0,0)

    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleClick does not let a black player select a white piece") {
    GameState st = makeState({"wK . . .", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::Black); // white king at (0,0)

    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleClick ignores a click from the wrong color while a selection is active") {
    GameState st = makeState({"wR . . bN", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::White); // white selects its rook at (0,0)
    REQUIRE(st.selection.active);
    REQUIRE(st.selection.cell.col == 0);

    // Black tries to act on white's pending selection - must be a complete
    // no-op: no reselect, no move, no cancellation of white's selection.
    Controller::click(st, 305, 5, Color::Black); // black's own knight at (0,3)

    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 0);           // still white's rook
    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");    // rook never moved
    CHECK(tokenAt(st.board, {0, 3}) == "bN");    // knight never captured/moved
}

TEST_CASE("handleClick ignores clicks outside the board") {
    GameState st = makeState({"wK ."});
    Controller::click(st, -5, -5, Color::White);
    Controller::click(st, 10000, 10000, Color::White);

    CHECK_FALSE(st.selection.active);
}

TEST_CASE("handleClick cancels an active selection on an outside-board click") {
    GameState st = makeState({"wR . . bN", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::White); // white selects the rook at (0,0)
    REQUIRE(st.selection.active);

    Controller::click(st, -5, -5, Color::White); // white, outside the board -> cancels its own selection

    CHECK_FALSE(st.selection.active);

    // The next click - by black now, with no selection pending - must open a
    // fresh selection on its own knight, not be misread as completing a
    // move from the stale (already-cleared) selection.
    Controller::click(st, 305, 5, Color::Black); // black selects its own knight at (0,3)

    CHECK_FALSE(st.arbiter.hasActiveMotion());
    CHECK(tokenAt(st.board, {0, 0}) == "wR");   // rook never moved
    CHECK(st.selection.active);
    CHECK(st.selection.cell.col == 3);          // freshly selected the knight, not a completed move
}

TEST_CASE("handleClick does not let an outside-board click from the wrong color cancel someone else's selection") {
    GameState st = makeState({"wR . . bN", ". . . .", ". . . .", ". . . ."});

    Controller::click(st, 5, 5, Color::White); // white selects its rook
    REQUIRE(st.selection.active);

    Controller::click(st, -5, -5, Color::Black); // black's out-of-bounds click

    CHECK(st.selection.active);   // white's selection survives - it wasn't black's to cancel
}

TEST_CASE("handleClick on an empty cell with no pending selection is a no-op") {
    GameState st = makeState({". . .", ". . .", ". . ."});
    Controller::click(st, 5, 5, Color::White);
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

TEST_CASE("runCommands demonstrates a pawn's two-square opening move with a clear path") {
    // 4 rows so the landing square (row 1) is NOT the last row - this test is
    // about the two-square move itself, not promotion, so it must land the
    // pawn somewhere it stays a pawn.
    GameState st = makeState({". . .", ". . .", ". . .", "wP . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "click 5 305",   // select the pawn at (3,0)
        "click 5 105",   // move it two squares to (1,0)
        "wait 2000",     // pawn: 1.0 cells/sec, 2 cells travelled -> 2000ms
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {1, 0}) == "wP");
}

TEST_CASE("runCommands demonstrates a pawn promoting to a queen upon reaching the last row") {
    GameState st = makeState({". . .", "wP . .", ". . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "click 5 105",   // select the pawn at (1,0), one square from the last row
        "click 5 5",     // move it to (0,0)
        "wait 1000",     // pawn: 1.0 cells/sec, 1 cell travelled -> 1000ms
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {0, 0}) == "wQ");
}

TEST_CASE("runCommands demonstrates a successful jump interception") {
    GameState st = makeState({"wR bR .", ". . .", ". . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "jump 5 5",      // wR at (0,0) jumps in place
        "click 105 5 black", // select bR at (0,1)
        "click 5 5 black",   // send it toward wR's cell - a legal 1-cell rook move
        "wait 1000",     // rook: 1.0 cells/sec, 1 cell -> 1000ms - same tick as the jump's window
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {0, 0}) == "wR");          // jumper survives, unmoved
    CHECK(tokenAt(st.board, {0, 1}) == EMPTY_TOKEN);   // arriving enemy destroyed mid-air
    CHECK_FALSE(st.arbiter.hasActiveJump());
}

TEST_CASE("runCommands demonstrates a jump landing normally with no interception") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "jump 5 5",      // wR at (0,0) jumps in place
        "wait 1000",     // no enemy arrives - the jump simply lands
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {0, 0}) == "wR");
    CHECK_FALSE(st.arbiter.hasActiveJump());
}

TEST_CASE("runCommands demonstrates a rejected move attempt against a currently-jumping piece") {
    GameState st = makeState({"wR . .", ". . .", ". . ."});

    std::ostringstream captured;
    std::streambuf* old = std::cout.rdbuf(captured.rdbuf());

    std::vector<std::string> commands = {
        "jump 5 5",      // wR at (0,0) starts jumping
        "click 5 5",     // select the jumping rook itself
        "click 205 5",   // attempt to move it - must be rejected outright
        "wait 1000",     // the jump lands normally in the meantime
        "print board"
    };
    ScriptRunner::run(commands, st);

    std::cout.rdbuf(old);

    CHECK(captured.str() == formatBoard(st.board));
    CHECK(tokenAt(st.board, {0, 0}) == "wR");   // never moved
    CHECK_FALSE(st.arbiter.hasActiveMotion());
}
