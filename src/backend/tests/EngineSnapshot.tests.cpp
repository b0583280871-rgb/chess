#include "doctest.h"

#include "../engine/GameEngine.hpp"

#include "../io/BoardParser.hpp"

TEST_CASE("GameEngine::snapshot interpolates a piece's pixel position mid-motion") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});

    PieceMove m;
    m.from = {0, 0};
    m.to = {0, 1};
    m.startMs = 0;
    m.durationMs = 1000;
    m.piece = "wR";
    st.arbiter.startMotion(m);

    st.elapsedMs = 500;   // 50% progress

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.pieces.size() == 1);
    const PieceSnapshot& rook = snap.pieces[0];

    CHECK(rook.pixelX == 50);   // halfway between col0 (x=0) and col1 (x=100)
    CHECK(rook.pixelY == 0);    // same row throughout
    CHECK(rook.animState == "move");
}

TEST_CASE("GameEngine::snapshot returns an empty pieces vector for an empty board") {
    GameState st;
    st.board = parseBoard({". . .", ". . .", ". . ."});

    GameSnapshot snap = GameEngine::snapshot(st);

    CHECK(snap.rows == 3);
    CHECK(snap.cols == 3);
    CHECK(snap.pieces.empty());
}

TEST_CASE("GameEngine::snapshot reports the correct pieceCode and pixel position for each piece") {
    GameState st;
    st.board = parseBoard({
        "wR . .",
        ". bQ .",
        ". . wK"
    });

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.pieces.size() == 3);

    auto findByCode = [&](const std::string& code) -> const PieceSnapshot* {
        for (const auto& p : snap.pieces) {
            if (p.pieceCode == code) return &p;
        }
        return nullptr;
    };

    const PieceSnapshot* rook = findByCode("RW");
    REQUIRE(rook != nullptr);
    CHECK(rook->pixelX == 0);
    CHECK(rook->pixelY == 0);
    CHECK(rook->animState == "idle");

    const PieceSnapshot* queen = findByCode("QB");
    REQUIRE(queen != nullptr);
    CHECK(queen->pixelX == 100);
    CHECK(queen->pixelY == 100);

    const PieceSnapshot* king = findByCode("KW");
    REQUIRE(king != nullptr);
    CHECK(king->pixelX == 200);
    CHECK(king->pixelY == 200);
}

TEST_CASE("GameEngine::snapshot reports selectedCell when a selection is active") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});
    st.selection = {true, {1, 2}, 0};

    GameSnapshot snap = GameEngine::snapshot(st);

    REQUIRE(snap.selectedCell.has_value());
    CHECK(snap.selectedCell->row == 1);
    CHECK(snap.selectedCell->col == 2);
}

TEST_CASE("GameEngine::snapshot reports nullopt selectedCell when there is no active selection") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});
    // selection.active defaults to false

    GameSnapshot snap = GameEngine::snapshot(st);

    CHECK_FALSE(snap.selectedCell.has_value());
}

TEST_CASE("GameEngine::snapshot passes the gameOver flag through unchanged") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});

    st.gameOver = false;
    CHECK_FALSE(GameEngine::snapshot(st).gameOver);

    st.gameOver = true;
    CHECK(GameEngine::snapshot(st).gameOver);
}

TEST_CASE("GameEngine::snapshot skips a piece whose state is Captured") {
    GameState st;
    st.board = parseBoard({". . .", ". . .", ". . ."});

    Piece captured;
    captured.id = 1;
    captured.color = Color::White;
    captured.kind = Kind::Pawn;
    captured.cell = {0, 0};
    captured.state = PieceState::Captured;
    st.board.addPiece(captured);

    GameSnapshot snap = GameEngine::snapshot(st);

    CHECK(snap.pieces.empty());
}
