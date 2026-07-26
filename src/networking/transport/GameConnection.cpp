#include "GameConnection.hpp"

#include <iostream>

#include "networking/protocol/JsonCodec.hpp"

GameConnection::GameConnection(const std::string& uri) : uri_(uri) {
}

bool GameConnection::connect() {
    client_.set_access_channels(websocketpp::log::alevel::none);
    client_.clear_access_channels(websocketpp::log::alevel::all);

    client_.init_asio();

    client_.set_open_handler([this](connection_hdl hdl) {
        hdl_ = hdl;
        connected_ = true;
        std::cout << "Connected to game_server." << std::endl;
    });

    client_.set_message_handler([this](connection_hdl, WsClient::message_ptr msg) {
        const std::string rawText = msg->get_payload();

        std::string type;
        try {
            type = protocol::envelopeType(rawText);
        } catch (const std::exception& e) {
            std::cout << "Ignoring malformed message: " << e.what() << std::endl;
            return;
        }

        try {
            nlohmann::json parsed = nlohmann::json::parse(rawText);
            nlohmann::json payload = parsed.at("payload");
            if (messageHandler_) {
                messageHandler_(type, payload);
            }
        } catch (const std::exception& e) {
            std::cout << "Ignoring malformed message: " << e.what() << std::endl;
        }
    });

    client_.set_fail_handler([](connection_hdl) {
        std::cerr << "game_client: connection failed - is game_server running?" << std::endl;
    });

    websocketpp::lib::error_code ec;
    WsClient::connection_ptr con = client_.get_connection(uri_, ec);
    if (ec) {
        std::cerr << "game_client: could not create connection: " << ec.message() << std::endl;
        return false;
    }

    client_.connect(con);
    return true;
}

void GameConnection::poll() {
    client_.poll();
}

bool GameConnection::isConnected() const {
    return connected_;
}

void GameConnection::send(const std::string& envelopeType, const nlohmann::json& payload) {
    nlohmann::json envelope = protocol::wrapEnvelope(envelopeType, payload);
    client_.send(hdl_, envelope.dump(), websocketpp::frame::opcode::text);
}

void GameConnection::sendClick(int x, int y) {
    protocol::ClickMessage click{x, y};
    nlohmann::json payload = click;
    send("click", payload);
}

void GameConnection::setMessageHandler(MessageHandler handler) {
    messageHandler_ = std::move(handler);
}

void GameConnection::close() {
    if (connected_) {
        client_.close(hdl_, websocketpp::close::status::normal, "client exiting");
        client_.poll();
    }
}
