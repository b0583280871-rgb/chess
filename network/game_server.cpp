
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <thread>

#include "../backend/engine/GameEngine.hpp"
#include "../backend/input/Controller.hpp"
#include "../backend/io/BoardFormat.hpp"
#include "../backend/io/BoardParser.hpp"
#include "SnapshotAdapter.hpp"
#include "protocol/JsonCodec.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

namespace {
    const uint16_t PORT = 9004;
    const char* ROOM_ID = "global";   // single global game - real room ids are a later step

    GameState buildInitialState() {
        const std::string boardText =
            io_format::SECTION_BOARD + "\n"
            "bR bN bB bQ bK bB bN bR\n"
            "bP bP bP bP bP bP bP bP\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            ". . . . . . . .\n"
            "wP wP wP wP wP wP wP wP\n"
            "wR wN wB wQ wK wB wN wR\n" +
            io_format::SECTION_COMMANDS + "\n";

        Sections sections = parseSections(boardText);

        GameState state;
        state.board = parseBoard(sections.boardLines);
        return state;
    }
}

int main() {
    server game_server;
    GameState state = buildInitialState();

    std::set<connection_hdl, std::owner_less<connection_hdl>> connections;
    std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> roles;

    try {
        game_server.set_access_channels(websocketpp::log::alevel::none);
        game_server.clear_access_channels(websocketpp::log::alevel::all);

        game_server.init_asio();

        game_server.set_open_handler([&](connection_hdl hdl) {
            std::string role;
            if (connections.empty()) {
                role = "white";
            } else if (connections.size() == 1) {
                role = "black";
            } else {
                role = "spectator";
                // TODO(S-future): S8 handles real spectator support (e.g.
                // read-only enforcement, late-join snapshot catch-up); for
                // now a third+ connection is just tracked so it keeps
                // receiving broadcasts, nothing else is special about it.
            }

            connections.insert(hdl);
            roles[hdl] = role;

            protocol::RoomJoinedMessage joined{ROOM_ID, role};
            nlohmann::json payload = joined;
            nlohmann::json envelope = protocol::wrapEnvelope("room_joined", payload);
            game_server.send(hdl, envelope.dump(), websocketpp::frame::opcode::text);

            std::cout << "Connection joined as: " << role << std::endl;
        });

        game_server.set_close_handler([&](connection_hdl hdl) {
            std::cout << "Connection closed (was: " << roles[hdl] << ")" << std::endl;
            connections.erase(hdl);
            roles.erase(hdl);
        });

        game_server.set_message_handler([&](connection_hdl /*hdl*/, server::message_ptr msg) {
            const std::string rawText = msg->get_payload();

            std::string type;
            try {
                type = protocol::envelopeType(rawText);
            } catch (const std::exception& e) {
                std::cout << "Ignoring malformed message: " << e.what() << std::endl;
                return;
            }

            if (type == "click") {
                nlohmann::json parsed = nlohmann::json::parse(rawText);
                protocol::ClickMessage click = parsed.at("payload").get<protocol::ClickMessage>();

                // TODO(S-future): enforce that the "white" connection can
                // only move white pieces (and likewise for "black") - not
                // enforced yet, every connection's click is passed straight
                // through to Controller::click as-is.
                Controller::click(state, click.x, click.y);
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            }
        });

        game_server.listen(PORT);
        game_server.start_accept();

        std::cout << "game_server listening on ws://localhost:" << PORT << std::endl;

        auto previousTime = std::chrono::steady_clock::now();

        while (true) {
            // TODO(S-future): single-threaded poll()-based loop is fine for
            // one global game with a handful of connections; real
            // concurrency/scaling across many simultaneous games is
            // deferred, per the instructor's own note that multi-user
            // scaling is future work. No threads or mutexes introduced here.
            game_server.poll();

            auto now = std::chrono::steady_clock::now();
            long deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime).count();
            previousTime = now;

            handleWait(state, deltaMs);

            GameSnapshot snap = GameEngine::snapshot(state);
            protocol::SnapshotMessage snapMsg = SnapshotAdapter::toProtocol(snap);
            nlohmann::json payload = snapMsg;
            nlohmann::json envelope = protocol::wrapEnvelope("snapshot", payload);
            std::string rawText = envelope.dump();

            for (const connection_hdl& hdl : connections) {
                websocketpp::lib::error_code ec;
                game_server.send(hdl, rawText, websocketpp::frame::opcode::text, ec);
                if (ec) {
                    std::cerr << "Broadcast send failed: " << ec.message() << std::endl;
                }
            }

            // Not part of the task's literal spec, added so this
            // non-blocking poll() loop doesn't pin a CPU core at 100% and
            // flood the network with thousands of identical snapshots per
            // second; ~33 snapshots/sec is plenty for this step.
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    } catch (websocketpp::exception const& e) {
        std::cerr << "game_server error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "game_server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
