#include "doctest.h"

#include "network/protocol/MessageTypes.hpp"
#include "network/protocol/Messages.hpp"

using namespace protocol;

// --- Client -> Server -------------------------------------------------

TEST_CASE("LoginMessage holds its fields") {
    LoginMessage msg{"alice", "hunter2"};
    CHECK(msg.username == "alice");
    CHECK(msg.password == "hunter2");
}

TEST_CASE("RegisterMessage holds its fields") {
    RegisterMessage msg{"alice@example.com", "hunter2"};
    CHECK(msg.email == "alice@example.com");
    CHECK(msg.password == "hunter2");
}

TEST_CASE("CreateRoomMessage holds its field") {
    CreateRoomMessage msg{"alices-room"};
    CHECK(msg.room_name == "alices-room");
}

TEST_CASE("JoinRoomMessage holds its field") {
    JoinRoomMessage msg{"alices-room"};
    CHECK(msg.room_name == "alices-room");
}

TEST_CASE("QuickPlayMessage constructs with no fields") {
    QuickPlayMessage msg;
    (void)msg;
}

TEST_CASE("ClickMessage holds its fields") {
    ClickMessage msg{320, 180};
    CHECK(msg.x == 320);
    CHECK(msg.y == 180);
}

TEST_CASE("LeaveMessage constructs with no fields") {
    LeaveMessage msg;
    (void)msg;
}

// --- Server -> Client ---------------------------------------------------

TEST_CASE("LoginResultMessage holds success and rating, no reason") {
    LoginResultMessage msg{true, std::nullopt, 1200};
    CHECK(msg.success);
    CHECK_FALSE(msg.reason.has_value());
    REQUIRE(msg.rating.has_value());
    CHECK(msg.rating.value() == 1200);
}

TEST_CASE("LoginResultMessage holds failure and reason, no rating") {
    LoginResultMessage msg{false, std::string("invalid_credentials"), std::nullopt};
    CHECK_FALSE(msg.success);
    REQUIRE(msg.reason.has_value());
    CHECK(msg.reason.value() == "invalid_credentials");
    CHECK_FALSE(msg.rating.has_value());
}

TEST_CASE("RegisterResultMessage holds success and an empty reason") {
    RegisterResultMessage msg{true, ""};
    CHECK(msg.success);
    CHECK(msg.reason.empty());
}

TEST_CASE("RegisterResultMessage holds failure and a reason") {
    RegisterResultMessage msg{false, "email_taken"};
    CHECK_FALSE(msg.success);
    CHECK(msg.reason == "email_taken");
}

TEST_CASE("RoomJoinedMessage holds its fields") {
    RoomJoinedMessage msg{"alices-room", "black"};
    CHECK(msg.room_id == "alices-room");
    CHECK(msg.role == "black");
}

TEST_CASE("MatchmakingResultMessage holds success and room_id") {
    MatchmakingResultMessage msg{true, std::string("room_42"), std::nullopt};
    CHECK(msg.success);
    REQUIRE(msg.room_id.has_value());
    CHECK(msg.room_id.value() == "room_42");
    CHECK_FALSE(msg.reason.has_value());
}

TEST_CASE("MatchmakingResultMessage holds failure and reason") {
    MatchmakingResultMessage msg{false, std::nullopt, std::string("no_opponent_found")};
    CHECK_FALSE(msg.success);
    CHECK_FALSE(msg.room_id.has_value());
    REQUIRE(msg.reason.has_value());
    CHECK(msg.reason.value() == "no_opponent_found");
}

TEST_CASE("SnapshotMessage holds rows, cols, pieces, selectedCell and gameOver") {
    SnapshotMessage msg;
    msg.rows = 8;
    msg.cols = 8;
    msg.pieces.push_back(SnapshotPieceMessage{"RW", 0, 0, "idle", 1});
    msg.selectedCell = SnapshotCell{1, 2};
    msg.gameOver = false;

    CHECK(msg.rows == 8);
    CHECK(msg.cols == 8);
    REQUIRE(msg.pieces.size() == 1);
    CHECK(msg.pieces[0].pieceCode == "RW");
    CHECK(msg.pieces[0].pixelX == 0);
    CHECK(msg.pieces[0].pixelY == 0);
    CHECK(msg.pieces[0].animState == "idle");
    CHECK(msg.pieces[0].frameIndex == 1);
    REQUIRE(msg.selectedCell.has_value());
    CHECK(msg.selectedCell->row == 1);
    CHECK(msg.selectedCell->col == 2);
    CHECK_FALSE(msg.gameOver);
}

TEST_CASE("SnapshotMessage selectedCell can be nullopt") {
    SnapshotMessage msg;
    msg.rows = 3;
    msg.cols = 3;
    msg.gameOver = true;

    CHECK_FALSE(msg.selectedCell.has_value());
    CHECK(msg.gameOver);
}

TEST_CASE("OpponentDisconnectedMessage holds its field") {
    OpponentDisconnectedMessage msg{30};
    CHECK(msg.countdown_seconds == 30);
}

TEST_CASE("GameOverResultMessage holds a winner and reason") {
    GameOverResultMessage msg{std::string("white"), "checkmate"};
    REQUIRE(msg.winner.has_value());
    CHECK(msg.winner.value() == "white");
    CHECK(msg.reason == "checkmate");
}

TEST_CASE("GameOverResultMessage supports a drawn game with no winner") {
    GameOverResultMessage msg{std::nullopt, "draw"};
    CHECK_FALSE(msg.winner.has_value());
    CHECK(msg.reason == "draw");
}

TEST_CASE("ErrorMessage holds its field") {
    ErrorMessage msg{"Room 'alices-room' not found"};
    CHECK(msg.message == "Room 'alices-room' not found");
}

TEST_CASE("MessageType enumerators are distinct values") {
    CHECK(MessageType::Login != MessageType::CreateRoom);
    CHECK(MessageType::Snapshot != MessageType::Error);
}
