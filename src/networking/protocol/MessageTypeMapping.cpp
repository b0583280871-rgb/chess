#include "MessageTypeMapping.hpp"

#include <stdexcept>
#include <utility>

namespace protocol
{

    namespace
    {
        constexpr std::pair<MessageType, const char *> kMessageTypeNames[] = {
            {MessageType::Login, "login"},
            {MessageType::Register, "register"},
            {MessageType::CreateRoom, "create_room"},
            {MessageType::JoinRoom, "join_room"},
            {MessageType::QuickPlay, "quick_play"},
            {MessageType::Click, "click"},
            {MessageType::Leave, "leave"},
            {MessageType::LoginResult, "login_result"},
            {MessageType::RegisterResult, "register_result"},
            {MessageType::RoomJoined, "room_joined"},
            {MessageType::MatchmakingResult, "matchmaking_result"},
            {MessageType::Snapshot, "snapshot"},
            {MessageType::OpponentDisconnected, "opponent_disconnected"},
            {MessageType::GameOverResult, "game_over_result"},
            {MessageType::Error, "error"},
        };
    }

    MessageType toMessageType(const std::string &name)
    {
        for (const auto &entry : kMessageTypeNames)
        {
            if (name == entry.second)
            {
                return entry.first;
            }
        }
        throw std::runtime_error("toMessageType: unknown message type '" + name + "'");
    }

    std::string toString(MessageType type)
    {
        for (const auto &entry : kMessageTypeNames)
        {
            if (type == entry.first)
            {
                return entry.second;
            }
        }
        throw std::runtime_error("toString: unrecognized MessageType value");
    }

}
