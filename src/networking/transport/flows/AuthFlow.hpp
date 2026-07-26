#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "../GameConnection.hpp"
#include "PendingResult.hpp"
#include "networking/protocol/MessageTypes.hpp"

enum class FlowState {
    Idle,
    WaitingForResponse,
};

class AuthFlow {
public:
    explicit AuthFlow(GameConnection& connection);

    void requestLogin(const std::string& email, const std::string& password);
    void requestRegister(const std::string& email, const std::string& password);

    void handleMessage(protocol::MessageType type, const nlohmann::json& payload);

    void acknowledgeResult();

    FlowState state() const;

    bool loginSucceeded() const;
    bool loginResultReceived() const;
    int loginRating() const;
    const std::string& loginFailureReason() const;

    bool registerResultReceived() const;
    bool registerSucceeded() const;
    const std::string& registerFailureReason() const;

private:
    struct LoginOutcome {
        bool success = false;
        int rating = 0;
        std::string failureReason;
    };

    struct RegisterOutcome {
        bool success = false;
        std::string failureReason;
    };

    void beginRequest();

    GameConnection& connection_;
    FlowState currentState_ = FlowState::Idle;

    PendingResult<LoginOutcome> loginResult_;
    PendingResult<RegisterOutcome> registerResult_;
};
