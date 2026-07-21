#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

#include "Messages.hpp"

namespace nlohmann {
template <typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json& j, const std::optional<T>& opt) {
        if (opt.has_value()) {
            j = *opt;
        } else {
            j = nullptr;
        }
    }
    static void from_json(const json& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        } else {
            opt = j.get<T>();
        }
    }
};
}

namespace protocol {


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LoginMessage, username, password)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RegisterMessage, email, password)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CreateRoomMessage, room_name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(JoinRoomMessage, room_name)

inline void to_json(nlohmann::json& j, const QuickPlayMessage&) {
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json&, QuickPlayMessage&) {
    // empty payload, nothing to read
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ClickMessage, x, y)

inline void to_json(nlohmann::json& j, const LeaveMessage&) {
    j = nlohmann::json::object();
}
inline void from_json(const nlohmann::json&, LeaveMessage&) {
    // empty payload, nothing to read
}


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LoginResultMessage, success, reason, rating)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RegisterResultMessage, success, reason)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RoomJoinedMessage, room_id, role)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MatchmakingResultMessage, success, room_id, reason)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SnapshotPieceMessage, pieceCode, pixelX, pixelY, animState, frameIndex)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SnapshotCell, row, col)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SnapshotMessage, rows, cols, pieces, selectedCell, gameOver)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OpponentDisconnectedMessage, countdown_seconds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(GameOverResultMessage, winner, reason)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ErrorMessage, message)

nlohmann::json wrapEnvelope(const std::string& typeName, const nlohmann::json& payload);

std::string envelopeType(const std::string& rawJsonText);

}
