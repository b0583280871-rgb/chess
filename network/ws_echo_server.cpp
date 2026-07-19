// Minimal WebSocket echo server - pure connectivity smoke test, no game
// logic. Listens on ws://localhost:9002 and echoes back any text message it
// receives, logging it to stdout first.

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

typedef websocketpp::server<websocketpp::config::asio> server;

namespace {
    const uint16_t PORT = 9002;
}

int main() {
    server echo_server;

    try {
        echo_server.set_access_channels(websocketpp::log::alevel::none);
        echo_server.clear_access_channels(websocketpp::log::alevel::all);

        echo_server.init_asio();

        echo_server.set_message_handler(
            [&echo_server](websocketpp::connection_hdl hdl, server::message_ptr msg) {
                std::cout << "Received: " << msg->get_payload() << std::endl;
                echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
            }
        );

        echo_server.listen(PORT);
        echo_server.start_accept();

        std::cout << "ws_echo_server listening on ws://localhost:" << PORT << std::endl;
        echo_server.run();
    } catch (websocketpp::exception const& e) {
        std::cerr << "ws_echo_server error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ws_echo_server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
