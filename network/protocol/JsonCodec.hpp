#pragma once

#include <nlohmann/json.hpp>

#include <string>

#include "Messages.hpp"

namespace protocol {


void to_json(nlohmann::json& j, const LoginMessage& m);
void from_json(const nlohmann::json& j, LoginMessage& m);

void to_json(nlohmann::json& j, const RegisterMessage& m);
void from_json(const nlohmann::json& j, RegisterMessage& m);

void to_json(nlohmann::json& j, const CreateRoomMessage& m);
void from_json(const nlohmann::json& j, CreateRoomMessage& m);

void to_json(nlohmann::json& j, const JoinRoomMessage& m);
void from_json(const nlohmann::json& j, JoinRoomMessage& m);

void to_json(nlohmann::json& j, const QuickPlayMessage& m);
void from_json(const nlohmann::json& j, QuickPlayMessage& m);

void to_json(nlohmann::json& j, const ClickMessage& m);
void from_json(const nlohmann::json& j, ClickMessage& m);

void to_json(nlohmann::json& j, const LeaveMessage& m);
void from_json(const nlohmann::json& j, LeaveMessage& m);




void to_json(nlohmann::json& j, const LoginResultMessage& m);
void from_json(const nlohmann::json& j, LoginResultMessage& m);

void to_json(nlohmann::json& j, const RegisterResultMessage& m);
void from_json(const nlohmann::json& j, RegisterResultMessage& m);

void to_json(nlohmann::json& j, const RoomJoinedMessage& m);
void from_json(const nlohmann::json& j, RoomJoinedMessage& m);

void to_json(nlohmann::json& j, const MatchmakingResultMessage& m);
void from_json(const nlohmann::json& j, MatchmakingResultMessage& m);

void to_json(nlohmann::json& j, const SnapshotPieceMessage& m);
void from_json(const nlohmann::json& j, SnapshotPieceMessage& m);

void to_json(nlohmann::json& j, const SnapshotCell& m);
void from_json(const nlohmann::json& j, SnapshotCell& m);

void to_json(nlohmann::json& j, const SnapshotMessage& m);
void from_json(const nlohmann::json& j, SnapshotMessage& m);

void to_json(nlohmann::json& j, const OpponentDisconnectedMessage& m);
void from_json(const nlohmann::json& j, OpponentDisconnectedMessage& m);

void to_json(nlohmann::json& j, const GameOverResultMessage& m);
void from_json(const nlohmann::json& j, GameOverResultMessage& m);

void to_json(nlohmann::json& j, const ErrorMessage& m);
void from_json(const nlohmann::json& j, ErrorMessage& m);


nlohmann::json wrapEnvelope(const std::string& typeName, const nlohmann::json& payload);

std::string envelopeType(const std::string& rawJsonText);

}
