
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>

#include "network/protocol/JsonCodec.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

namespace {
    const std::string URI = "ws://localhost:9003";
}

int main() {
    client c;

    try {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.clear_access_channels(websocketpp::log::alevel::all);

        c.init_asio();

        c.set_open_handler([&c](websocketpp::connection_hdl hdl) {
            protocol::LoginMessage login{"testuser", "testpass"};
            nlohmann::json payload = login;
            nlohmann::json envelope = protocol::wrapEnvelope("login", payload);
            std::string rawText = envelope.dump();

            std::cout << "Sending login for username: " << login.username << std::endl;
            c.send(hdl, rawText, websocketpp::frame::opcode::text);
        });

        c.set_message_handler([&c](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            const std::string rawText = msg->get_payload();
            std::string type = protocol::envelopeType(rawText);

            if (type == "login_result") {
                nlohmann::json parsed = nlohmann::json::parse(rawText);
                protocol::LoginResultMessage result = parsed.at("payload").get<protocol::LoginResultMessage>();

                std::cout << "Received login_result: success=" << std::boolalpha << result.success;
                if (result.rating.has_value()) {
                    std::cout << ", rating=" << result.rating.value();
                }
                if (result.reason.has_value()) {
                    std::cout << ", reason=" << result.reason.value();
                }
                std::cout << std::endl;
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            }

            c.close(hdl, websocketpp::close::status::normal, "done");
        });

        c.set_fail_handler([](websocketpp::connection_hdl) {
            std::cerr << "protocol_demo_client: connection failed - is protocol_demo_server running?" << std::endl;
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(URI, ec);
        if (ec) {
            std::cerr << "protocol_demo_client: could not create connection: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);
        c.run();
    } catch (websocketpp::exception const& e) {
        std::cerr << "protocol_demo_client error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "protocol_demo_client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
