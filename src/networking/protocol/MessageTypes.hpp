#pragma once

namespace protocol {

enum class MessageType {
    Login,
    Register,
    CreateRoom,
    JoinRoom,
    QuickPlay,
    Click,
    Leave,

    LoginResult,
    RegisterResult,
    RoomJoined,
    MatchmakingResult,
    Snapshot,
    OpponentDisconnected,
    GameOverResult,
    Error,
};

}
