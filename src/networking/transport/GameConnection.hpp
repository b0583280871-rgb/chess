#pragma once

#include <functional>
#include <string>

#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

class GameConnection {
public:
    using MessageHandler = std::function<void(const std::string& type, const nlohmann::json& payload)>;

    explicit GameConnection(const std::string& uri);

    bool connect();

    void poll();
    bool isConnected() const;

    void send(const std::string& envelopeType, const nlohmann::json& payload);
    void sendClick(int x, int y);

    void setMessageHandler(MessageHandler handler);

    void close();

private:
    typedef websocketpp::client<websocketpp::config::asio_client> WsClient;
    typedef websocketpp::connection_hdl connection_hdl;

    std::string uri_;
    WsClient client_;
    connection_hdl hdl_;
    bool connected_ = false;
    MessageHandler messageHandler_;
};
