// Minimal WebSocket echo client - pure connectivity smoke test, no game
// logic. Connects to ws://localhost:9002, sends one hardcoded text message,
// prints whatever comes back, then closes the connection.

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <string>

typedef websocketpp::client<websocketpp::config::asio_client> client;

namespace {
    const std::string URI = "ws://localhost:9002";
    const std::string MESSAGE = "hello server";
}

int main() {
    client c;

    try {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.clear_access_channels(websocketpp::log::alevel::all);

        c.init_asio();

        c.set_open_handler([&c](websocketpp::connection_hdl hdl) {
            std::cout << "Connected. Sending: " << MESSAGE << std::endl;
            c.send(hdl, MESSAGE, websocketpp::frame::opcode::text);
        });

        c.set_message_handler([&c](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            std::cout << "Received: " << msg->get_payload() << std::endl;
            c.close(hdl, websocketpp::close::status::normal, "done");
        });

        c.set_fail_handler([](websocketpp::connection_hdl) {
            std::cerr << "ws_echo_client: connection failed - is ws_echo_server running?" << std::endl;
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(URI, ec);
        if (ec) {
            std::cerr << "ws_echo_client: could not create connection: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);
        c.run();
    } catch (websocketpp::exception const& e) {
        std::cerr << "ws_echo_client error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ws_echo_client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
