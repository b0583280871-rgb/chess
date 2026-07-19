
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <optional>

#include "SnapshotAdapter.hpp"
#include "protocol/JsonCodec.hpp"
#include "view/Renderer.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::connection_hdl connection_hdl;

namespace {
    const std::string URI = "ws://localhost:9004";
    const std::string WINDOW_NAME = "Kung Fu Chess (networked)";

    struct ClientContext {
        client* c = nullptr;
        connection_hdl hdl;
        bool connected = false;
    };

    void onMouse(int event, int x, int y, int /*flags*/, void* userdata) {
        if (event != cv::EVENT_LBUTTONDOWN) return;

        ClientContext* ctx = static_cast<ClientContext*>(userdata);
        if (!ctx->connected) return;

        protocol::ClickMessage click{x, y};
        nlohmann::json payload = click;
        nlohmann::json envelope = protocol::wrapEnvelope("click", payload);

        ctx->c->send(ctx->hdl, envelope.dump(), websocketpp::frame::opcode::text);
    }
}

int main() {
    client c;
    ClientContext ctx;
    ctx.c = &c;

    std::optional<GameSnapshot> latestSnapshot;
    bool hasNewSnapshot = false;

    try {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.clear_access_channels(websocketpp::log::alevel::all);

        c.init_asio();

        c.set_open_handler([&](connection_hdl hdl) {
            ctx.hdl = hdl;
            ctx.connected = true;
            std::cout << "Connected to game_server." << std::endl;
        });

        c.set_message_handler([&](connection_hdl, client::message_ptr msg) {
            const std::string rawText = msg->get_payload();

            std::string type;
            try {
                type = protocol::envelopeType(rawText);
            } catch (const std::exception& e) {
                std::cout << "Ignoring malformed message: " << e.what() << std::endl;
                return;
            }

            nlohmann::json parsed = nlohmann::json::parse(rawText);

            if (type == "room_joined") {
                protocol::RoomJoinedMessage joined = parsed.at("payload").get<protocol::RoomJoinedMessage>();
                std::cout << "Assigned role: " << joined.role << " (room " << joined.room_id << ")" << std::endl;
            } else if (type == "snapshot") {
                protocol::SnapshotMessage snapMsg = parsed.at("payload").get<protocol::SnapshotMessage>();
                latestSnapshot = SnapshotAdapter::fromProtocol(snapMsg);
                hasNewSnapshot = true;
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            }
        });

        c.set_fail_handler([](connection_hdl) {
            std::cerr << "game_client: connection failed - is game_server running?" << std::endl;
        });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(URI, ec);
        if (ec) {
            std::cerr << "game_client: could not create connection: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);

        cv::namedWindow(WINDOW_NAME);
        cv::setMouseCallback(WINDOW_NAME, onMouse, static_cast<void*>(&ctx));

        bool gameOverKeySeen = false;

        while (true) {
            c.poll();

            if (hasNewSnapshot && latestSnapshot) {
                Img canvas = renderFrame(*latestSnapshot);
                cv::imshow(WINDOW_NAME, canvas.get_mat());
                hasNewSnapshot = false;
            }

            int key = cv::waitKey(1);

            if (key == 27 || key == 'q' || key == 'Q') {
                break;
            }

            if (latestSnapshot && latestSnapshot->gameOver) {
                if (gameOverKeySeen) break;
                if (key != -1) gameOverKeySeen = true;
            }
        }

        if (ctx.connected) {
            c.close(ctx.hdl, websocketpp::close::status::normal, "client exiting");
            c.poll();
        }

        cv::destroyAllWindows();
        return 0;
    } catch (websocketpp::exception const& e) {
        std::cerr << "game_client error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "game_client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
