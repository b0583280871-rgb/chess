#pragma once

#include <optional>
#include <string>
#include <vector>

namespace protocol {


struct LoginMessage {
    std::string username;
    std::string password;
};

struct CreateRoomMessage {
    std::string room_name;
};

struct JoinRoomMessage {
    std::string room_name;
};

struct QuickPlayMessage {
    // TODO
};

struct ClickMessage {
    int x;
    int y;
};

struct LeaveMessage {
    //TODO
};


struct LoginResultMessage {
    bool success;
    std::optional<std::string> reason;  
    std::optional<int> rating;          
};

struct RoomJoinedMessage {
    std::string room_id;
    std::string role;   
};

struct MatchmakingResultMessage {
    bool success;
    std::optional<std::string> room_id; 
    std::optional<std::string> reason;  
};

struct SnapshotPieceMessage {
    std::string pieceCode;
    int pixelX;
    int pixelY;
    std::string animState;
    int frameIndex;
};

struct SnapshotCell {
    int row;
    int col;
};

struct SnapshotMessage {
    int rows;
    int cols;
    std::vector<SnapshotPieceMessage> pieces;
    std::optional<SnapshotCell> selectedCell;
    bool gameOver;
};

struct OpponentDisconnectedMessage {
    int countdown_seconds;
};

struct GameOverResultMessage {
    std::optional<std::string> winner;   
    std::string reason;
};

struct ErrorMessage {
    std::string message;
};

}
