#include "doctest.h"

#include "view/GameSnapshot.hpp"

#include "../io/BoardParser.hpp"

TEST_CASE("buildSnapshot returns an empty pieces vector for an empty board") {
    GameState st;
    st.board = parseBoard({". . .", ". . .", ". . ."});

    GameSnapshot snap = buildSnapshot(st);

    CHECK(snap.rows == 3);
    CHECK(snap.cols == 3);
    CHECK(snap.pieces.empty());
}

TEST_CASE("buildSnapshot reports the correct pieceCode and pixel position for each piece") {
    GameState st;
    st.board = parseBoard({
        "wR . .",
        ". bQ .",
        ". . wK"
    });

    GameSnapshot snap = buildSnapshot(st);

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

TEST_CASE("buildSnapshot reports selectedCell when a selection is active") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});
    st.selection = {true, {1, 2}, 0};

    GameSnapshot snap = buildSnapshot(st);

    REQUIRE(snap.selectedCell.has_value());
    CHECK(snap.selectedCell->row == 1);
    CHECK(snap.selectedCell->col == 2);
}

TEST_CASE("buildSnapshot reports nullopt selectedCell when there is no active selection") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});
    // selection.active defaults to false

    GameSnapshot snap = buildSnapshot(st);

    CHECK_FALSE(snap.selectedCell.has_value());
}

TEST_CASE("buildSnapshot passes the gameOver flag through unchanged") {
    GameState st;
    st.board = parseBoard({"wR . .", ". . .", ". . ."});

    st.gameOver = false;
    CHECK_FALSE(buildSnapshot(st).gameOver);

    st.gameOver = true;
    CHECK(buildSnapshot(st).gameOver);
}

TEST_CASE("buildSnapshot skips a piece whose state is Captured") {
    GameState st;
    st.board = parseBoard({". . .", ". . .", ". . ."});

    Piece captured;
    captured.id = 1;
    captured.color = Color::White;
    captured.kind = Kind::Pawn;
    captured.cell = {0, 0};
    captured.state = PieceState::Captured;
    st.board.addPiece(captured);

    GameSnapshot snap = buildSnapshot(st);

    CHECK(snap.pieces.empty());
}
