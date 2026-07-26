#include "AuthFlow.hpp"

#include "networking/protocol/JsonCodec.hpp"

AuthFlow::AuthFlow(GameConnection& connection) : connection_(connection) {
}

void AuthFlow::beginRequest() {
    loginResult_.reset();
    registerResult_.reset();
    currentState_ = FlowState::WaitingForResponse;
}

void AuthFlow::requestLogin(const std::string& email, const std::string& password) {
    beginRequest();

    protocol::LoginMessage login{email, password};
    connection_.sendMessage(login);
}

void AuthFlow::requestRegister(const std::string& email, const std::string& password) {
    beginRequest();

    protocol::RegisterMessage reg{email, password};
    connection_.sendMessage(reg);
}

void AuthFlow::handleMessage(protocol::MessageType type, const nlohmann::json& payload) {
    if (type == protocol::MessageType::LoginResult) {
        protocol::LoginResultMessage result = payload.get<protocol::LoginResultMessage>();
        LoginOutcome outcome;
        outcome.success = result.success;
        if (result.success && result.rating.has_value()) {
            outcome.rating = result.rating.value();
        } else if (!result.success && result.reason.has_value()) {
            outcome.failureReason = result.reason.value();
        }
        loginResult_.set(outcome);
    } else if (type == protocol::MessageType::RegisterResult) {
        protocol::RegisterResultMessage result = payload.get<protocol::RegisterResultMessage>();
        RegisterOutcome outcome;
        outcome.success = result.success;
        outcome.failureReason = result.reason;
        registerResult_.set(outcome);
    }
}

void AuthFlow::acknowledgeResult() {
    currentState_ = FlowState::Idle;
}

FlowState AuthFlow::state() const {
    return currentState_;
}

bool AuthFlow::loginResultReceived() const {
    return loginResult_.received;
}

bool AuthFlow::loginSucceeded() const {
    return loginResult_.value.success;
}

int AuthFlow::loginRating() const {
    return loginResult_.value.rating;
}

const std::string& AuthFlow::loginFailureReason() const {
    return loginResult_.value.failureReason;
}

bool AuthFlow::registerResultReceived() const {
    return registerResult_.received;
}

bool AuthFlow::registerSucceeded() const {
    return registerResult_.value.success;
}

const std::string& AuthFlow::registerFailureReason() const {
    return registerResult_.value.failureReason;
}
