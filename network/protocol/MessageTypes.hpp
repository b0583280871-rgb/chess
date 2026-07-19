#pragma once

namespace protocol {

enum class MessageType {
    Login,
    CreateRoom,
    JoinRoom,
    QuickPlay,
    Click,
    Leave,

    LoginResult,
    RoomJoined,
    MatchmakingResult,
    Snapshot,
    OpponentDisconnected,
    GameOverResult,
    Error,
};

}
