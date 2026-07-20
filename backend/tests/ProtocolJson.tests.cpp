#include "doctest.h"

#include "network/protocol/JsonCodec.hpp"

using namespace protocol;
using nlohmann::json;

namespace {
    // Common round-trip shape shared by every test below: encode, wrap in
    // the envelope, decode the envelope's type, unwrap the payload, decode
    // back into a struct of type T.
    template <typename T>
    T roundTrip(const T& original, const std::string& typeName) {
        json payload = original;
        json envelope = wrapEnvelope(typeName, payload);

        std::string rawText = envelope.dump();

        CHECK(envelopeType(rawText) == typeName);

        json parsed = json::parse(rawText);
        T decoded = parsed.at("payload").get<T>();
        return decoded;
    }
}

// --- Client -> Server -------------------------------------------------

TEST_CASE("LoginMessage round-trips through JSON") {
    LoginMessage original{"alice", "hunter2"};
    LoginMessage decoded = roundTrip(original, "login");

    CHECK(decoded.username == original.username);
    CHECK(decoded.password == original.password);
}

TEST_CASE("RegisterMessage round-trips through JSON") {
    RegisterMessage original{"alice@example.com", "hunter2"};
    RegisterMessage decoded = roundTrip(original, "register");

    CHECK(decoded.email == original.email);
    CHECK(decoded.password == original.password);
}

TEST_CASE("CreateRoomMessage round-trips through JSON") {
    CreateRoomMessage original{"alices-room"};
    CreateRoomMessage decoded = roundTrip(original, "create_room");

    CHECK(decoded.room_name == original.room_name);
}

TEST_CASE("JoinRoomMessage round-trips through JSON") {
    JoinRoomMessage original{"alices-room"};
    JoinRoomMessage decoded = roundTrip(original, "join_room");

    CHECK(decoded.room_name == original.room_name);
}

TEST_CASE("QuickPlayMessage round-trips through JSON with an empty payload") {
    QuickPlayMessage original;
    json payload = original;
    json envelope = wrapEnvelope("quick_play", payload);
    std::string rawText = envelope.dump();

    CHECK(envelopeType(rawText) == "quick_play");

    json parsed = json::parse(rawText);
    CHECK(parsed.at("payload").is_object());
    CHECK(parsed.at("payload").empty());
    QuickPlayMessage decoded = parsed.at("payload").get<QuickPlayMessage>();
    (void)decoded;
}

TEST_CASE("ClickMessage round-trips through JSON") {
    ClickMessage original{320, 180};
    ClickMessage decoded = roundTrip(original, "click");

    CHECK(decoded.x == original.x);
    CHECK(decoded.y == original.y);
}

TEST_CASE("LeaveMessage round-trips through JSON with an empty payload") {
    LeaveMessage original;
    json payload = original;
    json envelope = wrapEnvelope("leave", payload);
    std::string rawText = envelope.dump();

    CHECK(envelopeType(rawText) == "leave");

    json parsed = json::parse(rawText);
    CHECK(parsed.at("payload").is_object());
    CHECK(parsed.at("payload").empty());
    LeaveMessage decoded = parsed.at("payload").get<LeaveMessage>();
    (void)decoded;
}

// --- Server -> Client ---------------------------------------------------

TEST_CASE("LoginResultMessage round-trips through JSON (success, with rating, no reason)") {
    LoginResultMessage original{true, std::nullopt, 1200};
    LoginResultMessage decoded = roundTrip(original, "login_result");

    CHECK(decoded.success == original.success);
    CHECK_FALSE(decoded.reason.has_value());
    REQUIRE(decoded.rating.has_value());
    CHECK(decoded.rating.value() == 1200);
}

TEST_CASE("LoginResultMessage round-trips through JSON (failure, with reason, no rating)") {
    LoginResultMessage original{false, std::string("invalid_credentials"), std::nullopt};
    LoginResultMessage decoded = roundTrip(original, "login_result");

    CHECK_FALSE(decoded.success);
    REQUIRE(decoded.reason.has_value());
    CHECK(decoded.reason.value() == "invalid_credentials");
    CHECK_FALSE(decoded.rating.has_value());
}

TEST_CASE("RegisterResultMessage round-trips through JSON (success)") {
    RegisterResultMessage original{true, ""};
    RegisterResultMessage decoded = roundTrip(original, "register_result");

    CHECK(decoded.success);
    CHECK(decoded.reason.empty());
}

TEST_CASE("RegisterResultMessage round-trips through JSON (failure, with reason)") {
    RegisterResultMessage original{false, "email_taken"};
    RegisterResultMessage decoded = roundTrip(original, "register_result");

    CHECK_FALSE(decoded.success);
    CHECK(decoded.reason == "email_taken");
}

TEST_CASE("RoomJoinedMessage round-trips through JSON") {
    RoomJoinedMessage original{"alices-room", "black"};
    RoomJoinedMessage decoded = roundTrip(original, "room_joined");

    CHECK(decoded.room_id == original.room_id);
    CHECK(decoded.role == original.role);
}

TEST_CASE("MatchmakingResultMessage round-trips through JSON (success, with room_id)") {
    MatchmakingResultMessage original{true, std::string("room_42"), std::nullopt};
    MatchmakingResultMessage decoded = roundTrip(original, "matchmaking_result");

    CHECK(decoded.success);
    REQUIRE(decoded.room_id.has_value());
    CHECK(decoded.room_id.value() == "room_42");
    CHECK_FALSE(decoded.reason.has_value());
}

TEST_CASE("MatchmakingResultMessage round-trips through JSON (failure, with reason)") {
    MatchmakingResultMessage original{false, std::nullopt, std::string("no_opponent_found")};
    MatchmakingResultMessage decoded = roundTrip(original, "matchmaking_result");

    CHECK_FALSE(decoded.success);
    CHECK_FALSE(decoded.room_id.has_value());
    REQUIRE(decoded.reason.has_value());
    CHECK(decoded.reason.value() == "no_opponent_found");
}

TEST_CASE("SnapshotMessage round-trips through JSON with a piece and a selected cell") {
    SnapshotMessage original;
    original.rows = 8;
    original.cols = 8;
    original.pieces.push_back(SnapshotPieceMessage{"RW", 0, 0, "idle", 1});
    original.selectedCell = SnapshotCell{1, 2};
    original.gameOver = false;

    SnapshotMessage decoded = roundTrip(original, "snapshot");

    CHECK(decoded.rows == original.rows);
    CHECK(decoded.cols == original.cols);
    REQUIRE(decoded.pieces.size() == 1);
    CHECK(decoded.pieces[0].pieceCode == "RW");
    CHECK(decoded.pieces[0].pixelX == 0);
    CHECK(decoded.pieces[0].pixelY == 0);
    CHECK(decoded.pieces[0].animState == "idle");
    CHECK(decoded.pieces[0].frameIndex == 1);
    REQUIRE(decoded.selectedCell.has_value());
    CHECK(decoded.selectedCell->row == 1);
    CHECK(decoded.selectedCell->col == 2);
    CHECK_FALSE(decoded.gameOver);
}

TEST_CASE("SnapshotMessage round-trips through JSON with selectedCell explicitly null") {
    SnapshotMessage original;
    original.rows = 3;
    original.cols = 3;
    original.gameOver = true;

    SnapshotMessage decoded = roundTrip(original, "snapshot");

    CHECK(decoded.rows == 3);
    CHECK(decoded.cols == 3);
    CHECK(decoded.pieces.empty());
    CHECK_FALSE(decoded.selectedCell.has_value());
    CHECK(decoded.gameOver);
}

TEST_CASE("OpponentDisconnectedMessage round-trips through JSON") {
    OpponentDisconnectedMessage original{30};
    OpponentDisconnectedMessage decoded = roundTrip(original, "opponent_disconnected");

    CHECK(decoded.countdown_seconds == original.countdown_seconds);
}

TEST_CASE("GameOverResultMessage round-trips through JSON with a winner") {
    GameOverResultMessage original{std::string("white"), "checkmate"};
    GameOverResultMessage decoded = roundTrip(original, "game_over_result");

    REQUIRE(decoded.winner.has_value());
    CHECK(decoded.winner.value() == "white");
    CHECK(decoded.reason == "checkmate");
}

TEST_CASE("GameOverResultMessage round-trips through JSON with a null winner (draw)") {
    GameOverResultMessage original{std::nullopt, "draw"};
    GameOverResultMessage decoded = roundTrip(original, "game_over_result");

    CHECK_FALSE(decoded.winner.has_value());
    CHECK(decoded.reason == "draw");
}

TEST_CASE("ErrorMessage round-trips through JSON") {
    ErrorMessage original{"Room 'alices-room' not found"};
    ErrorMessage decoded = roundTrip(original, "error");

    CHECK(decoded.message == original.message);
}

// --- envelopeType error handling ----------------------------------------

TEST_CASE("envelopeType throws on text that isn't valid JSON at all") {
    CHECK_THROWS_AS(envelopeType("not json"), std::runtime_error);
}

TEST_CASE("envelopeType throws when the 'type' field is missing") {
    CHECK_THROWS_AS(envelopeType(R"({"payload": {}})"), std::runtime_error);
}

TEST_CASE("envelopeType throws when 'type' is present but not a string") {
    CHECK_THROWS_AS(envelopeType(R"({"type": 42, "payload": {}})"), std::runtime_error);
}

TEST_CASE("envelopeType returns the type field for a well-formed envelope") {
    CHECK(envelopeType(R"({"type": "login", "payload": {"username": "a", "password": "b"}})") == "login");
}
