#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "GameConnection.hpp"

enum class FlowState {
    Idle,
    WaitingForResponse,
};

class AuthFlow {
public:
    explicit AuthFlow(GameConnection& connection);

    void requestLogin(const std::string& email, const std::string& password);
    void requestRegister(const std::string& email, const std::string& password);

    void handleMessage(const std::string& type, const nlohmann::json& payload);

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
    GameConnection& connection_;
    FlowState currentState_ = FlowState::Idle;

    bool loginResultReceived_ = false;
    bool loginSucceeded_ = false;
    std::string loginFailureReason_;
    int loginRating_ = 0;

    bool registerResultReceived_ = false;
    bool registerSucceeded_ = false;
    std::string registerFailureReason_;
};
