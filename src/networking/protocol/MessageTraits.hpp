#pragma once

#include "MessageTypes.hpp"
#include "Messages.hpp"

// Compile-time struct-type -> MessageType mapping, used by
// GameConnection::sendMessage<TMsg>() to look up the wire type name for a
// given payload struct without GameConnection needing to know that struct
// exists. Deliberately independent of MessageTypeMapping.hpp's runtime
// string<->enum lookup - this only ever needs to go from a C++ type (known
// at compile time) to a MessageType value.
namespace protocol
{

    template <typename T>
    struct MessageTraits;

#define DEFINE_MESSAGE_TRAITS(StructName, EnumValue) \
    template <> \
    struct MessageTraits<StructName> { \
        static constexpr MessageType kind = MessageType::EnumValue; \
    };

    DEFINE_MESSAGE_TRAITS(LoginMessage, Login)
    DEFINE_MESSAGE_TRAITS(RegisterMessage, Register)
    DEFINE_MESSAGE_TRAITS(CreateRoomMessage, CreateRoom)
    DEFINE_MESSAGE_TRAITS(JoinRoomMessage, JoinRoom)
    DEFINE_MESSAGE_TRAITS(QuickPlayMessage, QuickPlay)
    DEFINE_MESSAGE_TRAITS(ClickMessage, Click)
    DEFINE_MESSAGE_TRAITS(LeaveMessage, Leave)
    DEFINE_MESSAGE_TRAITS(LoginResultMessage, LoginResult)
    DEFINE_MESSAGE_TRAITS(RegisterResultMessage, RegisterResult)
    DEFINE_MESSAGE_TRAITS(RoomJoinedMessage, RoomJoined)
    DEFINE_MESSAGE_TRAITS(MatchmakingResultMessage, MatchmakingResult)
    DEFINE_MESSAGE_TRAITS(SnapshotMessage, Snapshot)
    DEFINE_MESSAGE_TRAITS(OpponentDisconnectedMessage, OpponentDisconnected)
    DEFINE_MESSAGE_TRAITS(GameOverResultMessage, GameOverResult)
    DEFINE_MESSAGE_TRAITS(ErrorMessage, Error)

#undef DEFINE_MESSAGE_TRAITS

}
