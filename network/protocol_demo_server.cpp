
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

#include "network/protocol/JsonCodec.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;

namespace {
    const uint16_t PORT = 9003;
}

int main() {
    server demo_server;

    try {
        demo_server.set_access_channels(websocketpp::log::alevel::none);
        demo_server.clear_access_channels(websocketpp::log::alevel::all);

        demo_server.init_asio();

        demo_server.set_message_handler(
            [&demo_server](websocketpp::connection_hdl hdl, server::message_ptr msg) {
                const std::string rawText = msg->get_payload();

                std::string type;
                try {
                    type = protocol::envelopeType(rawText);
                } catch (const std::exception& e) {
                    std::cout << "Ignoring malformed message: " << e.what() << std::endl;
                    return;
                }

                if (type == "login") {
                    nlohmann::json parsed = nlohmann::json::parse(rawText);
                    protocol::LoginMessage login = parsed.at("payload").get<protocol::LoginMessage>();

                    std::cout << "Received login for username: " << login.username << std::endl;
                    // Deliberately not logging login.password, even in a throwaway demo.

                    protocol::LoginResultMessage result{true, std::nullopt, 1200};
                    nlohmann::json payload = result;
                    nlohmann::json envelope = protocol::wrapEnvelope("login_result", payload);

                    demo_server.send(hdl, envelope.dump(), websocketpp::frame::opcode::text);
                } else {
                    std::cout << "unhandled type: " << type << std::endl;
                }
            }
        );

        demo_server.listen(PORT);
        demo_server.start_accept();

        std::cout << "protocol_demo_server listening on ws://localhost:" << PORT << std::endl;
        demo_server.run();
    } catch (websocketpp::exception const& e) {
        std::cerr << "protocol_demo_server error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "protocol_demo_server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
