#include "JsonCodec.hpp"

#include <stdexcept>

namespace {

template <typename T>
void setOmitIfEmpty(nlohmann::json& j, const std::string& key, const std::optional<T>& value) {
    if (value.has_value()) {
        j[key] = *value;
    }
}

template <typename T>
std::optional<T> getOmittable(const nlohmann::json& j, const std::string& key) {
    if (j.contains(key)) {
        return j.at(key).get<T>();
    }
    return std::nullopt;
}

template <typename T>
void setAsNullable(nlohmann::json& j, const std::string& key, const std::optional<T>& value) {
    j[key] = value.has_value() ? nlohmann::json(*value) : nlohmann::json(nullptr);
}

template <typename T>
std::optional<T> getNullable(const nlohmann::json& j, const std::string& key) {
    if (!j.contains(key) || j.at(key).is_null()) {
        return std::nullopt;
    }
    return j.at(key).get<T>();
}

}

namespace protocol {


void to_json(nlohmann::json& j, const LoginMessage& m) {
    j = nlohmann::json{{"username", m.username}, {"password", m.password}};
}
void from_json(const nlohmann::json& j, LoginMessage& m) {
    j.at("username").get_to(m.username);
    j.at("password").get_to(m.password);
}

void to_json(nlohmann::json& j, const CreateRoomMessage& m) {
    j = nlohmann::json{{"room_name", m.room_name}};
}
void from_json(const nlohmann::json& j, CreateRoomMessage& m) {
    j.at("room_name").get_to(m.room_name);
}

void to_json(nlohmann::json& j, const JoinRoomMessage& m) {
    j = nlohmann::json{{"room_name", m.room_name}};
}
void from_json(const nlohmann::json& j, JoinRoomMessage& m) {
    j.at("room_name").get_to(m.room_name);
}

void to_json(nlohmann::json& j, const QuickPlayMessage&) {
    j = nlohmann::json::object();
}
void from_json(const nlohmann::json&, QuickPlayMessage&) {
    // empty payload, nothing to read
}

void to_json(nlohmann::json& j, const ClickMessage& m) {
    j = nlohmann::json{{"x", m.x}, {"y", m.y}};
}
void from_json(const nlohmann::json& j, ClickMessage& m) {
    j.at("x").get_to(m.x);
    j.at("y").get_to(m.y);
}

void to_json(nlohmann::json& j, const LeaveMessage&) {
    j = nlohmann::json::object();
}
void from_json(const nlohmann::json&, LeaveMessage&) {
    // empty payload, nothing to read
}


void to_json(nlohmann::json& j, const LoginResultMessage& m) {
    j = nlohmann::json{{"success", m.success}};
    setOmitIfEmpty(j, "reason", m.reason);
    setOmitIfEmpty(j, "rating", m.rating);
}
void from_json(const nlohmann::json& j, LoginResultMessage& m) {
    j.at("success").get_to(m.success);
    m.reason = getOmittable<std::string>(j, "reason");
    m.rating = getOmittable<int>(j, "rating");
}

void to_json(nlohmann::json& j, const RoomJoinedMessage& m) {
    j = nlohmann::json{{"room_id", m.room_id}, {"role", m.role}};
}
void from_json(const nlohmann::json& j, RoomJoinedMessage& m) {
    j.at("room_id").get_to(m.room_id);
    j.at("role").get_to(m.role);
}

void to_json(nlohmann::json& j, const MatchmakingResultMessage& m) {
    j = nlohmann::json{{"success", m.success}};
    setOmitIfEmpty(j, "room_id", m.room_id);
    setOmitIfEmpty(j, "reason", m.reason);
}
void from_json(const nlohmann::json& j, MatchmakingResultMessage& m) {
    j.at("success").get_to(m.success);
    m.room_id = getOmittable<std::string>(j, "room_id");
    m.reason = getOmittable<std::string>(j, "reason");
}

void to_json(nlohmann::json& j, const SnapshotPieceMessage& m) {
    j = nlohmann::json{
        {"pieceCode", m.pieceCode},
        {"pixelX", m.pixelX},
        {"pixelY", m.pixelY},
        {"animState", m.animState},
        {"frameIndex", m.frameIndex}
    };
}
void from_json(const nlohmann::json& j, SnapshotPieceMessage& m) {
    j.at("pieceCode").get_to(m.pieceCode);
    j.at("pixelX").get_to(m.pixelX);
    j.at("pixelY").get_to(m.pixelY);
    j.at("animState").get_to(m.animState);
    j.at("frameIndex").get_to(m.frameIndex);
}

void to_json(nlohmann::json& j, const SnapshotCell& m) {
    j = nlohmann::json{{"row", m.row}, {"col", m.col}};
}
void from_json(const nlohmann::json& j, SnapshotCell& m) {
    j.at("row").get_to(m.row);
    j.at("col").get_to(m.col);
}

void to_json(nlohmann::json& j, const SnapshotMessage& m) {
    j = nlohmann::json{
        {"rows", m.rows},
        {"cols", m.cols},
        {"pieces", m.pieces},
        {"gameOver", m.gameOver}
    };
    setAsNullable(j, "selectedCell", m.selectedCell);
}
void from_json(const nlohmann::json& j, SnapshotMessage& m) {
    j.at("rows").get_to(m.rows);
    j.at("cols").get_to(m.cols);
    j.at("pieces").get_to(m.pieces);
    j.at("gameOver").get_to(m.gameOver);
    m.selectedCell = getNullable<SnapshotCell>(j, "selectedCell");
}

void to_json(nlohmann::json& j, const OpponentDisconnectedMessage& m) {
    j = nlohmann::json{{"countdown_seconds", m.countdown_seconds}};
}
void from_json(const nlohmann::json& j, OpponentDisconnectedMessage& m) {
    j.at("countdown_seconds").get_to(m.countdown_seconds);
}

void to_json(nlohmann::json& j, const GameOverResultMessage& m) {
    j = nlohmann::json{{"reason", m.reason}};
    setAsNullable(j, "winner", m.winner);
}
void from_json(const nlohmann::json& j, GameOverResultMessage& m) {
    j.at("reason").get_to(m.reason);
    m.winner = getNullable<std::string>(j, "winner");
}

void to_json(nlohmann::json& j, const ErrorMessage& m) {
    j = nlohmann::json{{"message", m.message}};
}
void from_json(const nlohmann::json& j, ErrorMessage& m) {
    j.at("message").get_to(m.message);
}

// --- Envelope helpers ---------------------------------------------------

nlohmann::json wrapEnvelope(const std::string& typeName, const nlohmann::json& payload) {
    return nlohmann::json{{"type", typeName}, {"payload", payload}};
}

std::string envelopeType(const std::string& rawJsonText) {
    nlohmann::json parsed;
    try {
        parsed = nlohmann::json::parse(rawJsonText);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(std::string("envelopeType: invalid JSON: ") + e.what());
    }

    if (!parsed.contains("type") || !parsed.at("type").is_string()) {
        throw std::runtime_error("envelopeType: missing or non-string 'type' field");
    }

    return parsed.at("type").get<std::string>();
}

}
