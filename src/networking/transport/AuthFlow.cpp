#include "AuthFlow.hpp"

#include "networking/protocol/JsonCodec.hpp"

AuthFlow::AuthFlow(GameConnection& connection) : connection_(connection) {
}

void AuthFlow::requestLogin(const std::string& email, const std::string& password) {
    loginResultReceived_ = false;
    registerResultReceived_ = false;

    protocol::LoginMessage login{email, password};
    nlohmann::json payload = login;
    connection_.send("login", payload);

    currentState_ = FlowState::WaitingForResponse;
}

void AuthFlow::requestRegister(const std::string& email, const std::string& password) {
    loginResultReceived_ = false;
    registerResultReceived_ = false;

    protocol::RegisterMessage reg{email, password};
    nlohmann::json payload = reg;
    connection_.send("register", payload);

    currentState_ = FlowState::WaitingForResponse;
}

void AuthFlow::handleMessage(const std::string& type, const nlohmann::json& payload) {
    if (type == "login_result") {
        protocol::LoginResultMessage result = payload.get<protocol::LoginResultMessage>();
        loginResultReceived_ = true;
        loginSucceeded_ = result.success;
        if (result.success && result.rating.has_value()) {
            loginRating_ = result.rating.value();
        } else if (!result.success && result.reason.has_value()) {
            loginFailureReason_ = result.reason.value();
        }
    } else if (type == "register_result") {
        protocol::RegisterResultMessage result = payload.get<protocol::RegisterResultMessage>();
        registerResultReceived_ = true;
        registerSucceeded_ = result.success;
        registerFailureReason_ = result.reason;
    }
}

void AuthFlow::acknowledgeResult() {
    currentState_ = FlowState::Idle;
}

FlowState AuthFlow::state() const {
    return currentState_;
}

bool AuthFlow::loginResultReceived() const {
    return loginResultReceived_;
}

bool AuthFlow::loginSucceeded() const {
    return loginSucceeded_;
}

int AuthFlow::loginRating() const {
    return loginRating_;
}

const std::string& AuthFlow::loginFailureReason() const {
    return loginFailureReason_;
}

bool AuthFlow::registerResultReceived() const {
    return registerResultReceived_;
}

bool AuthFlow::registerSucceeded() const {
    return registerSucceeded_;
}

const std::string& AuthFlow::registerFailureReason() const {
    return registerFailureReason_;
}
