
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

#include "../backend/engine/GameEngine.hpp"
#include "../backend/input/Controller.hpp"
#include "../backend/io/BoardFormat.hpp"
#include "../backend/io/BoardParser.hpp"
#include "../persistence/Database.hpp"
#include "../persistence/UserRepository.hpp"
#include "SnapshotAdapter.hpp"
#include "protocol/JsonCodec.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

namespace {
    const uint16_t PORT = 9004;
    const char* ROOM_ID = "global";         
    const char* USERS_DB_PATH = "chess_users.db";

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

    struct ConnectionInfo {
        bool loggedIn = false;
        std::string email;
        std::string role;   // "white" | "black" | "spectator", set once loggedIn
    };

    Color colorFromRole(const std::string& role) {
        return (role == "white") ? Color::White : Color::Black;
    }
}

int main() {
    server game_server;
    GameState state = buildInitialState();
    Database userDb(USERS_DB_PATH);

    std::map<connection_hdl, ConnectionInfo, std::owner_less<connection_hdl>> connectionInfos;

    auto countLoggedIn = [&]() {
        int count = 0;
        for (const auto& entry : connectionInfos) {
            if (entry.second.loggedIn) ++count;
        }
        return count;
    };

    auto sendTo = [&](connection_hdl hdl, const std::string& typeName, const nlohmann::json& payload) {
        nlohmann::json envelope = protocol::wrapEnvelope(typeName, payload);
        game_server.send(hdl, envelope.dump(), websocketpp::frame::opcode::text);
    };

    try {
        game_server.set_access_channels(websocketpp::log::alevel::none);
        game_server.clear_access_channels(websocketpp::log::alevel::all);

        game_server.init_asio();

        game_server.set_open_handler([&](connection_hdl hdl) {
            connectionInfos[hdl] = ConnectionInfo{};
            std::cout << "Connection opened, awaiting login." << std::endl;
        });

        game_server.set_close_handler([&](connection_hdl hdl) {
            auto it = connectionInfos.find(hdl);
            std::string who = (it != connectionInfos.end() && it->second.loggedIn)
                ? it->second.email : "(not logged in)";
            std::cout << "Connection closed (was: " << who << ")" << std::endl;
            connectionInfos.erase(hdl);
        });

        game_server.set_message_handler([&](connection_hdl hdl, server::message_ptr msg) {
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

                // Never log the password - only the email and the outcome.
                std::cout << "Login attempt for email: " << login.username << std::endl;

                LoginResult result = loginUser(userDb, login.username, login.password);

                protocol::LoginResultMessage resultMsg;
                resultMsg.success = result.success;
                if (result.success) {
                    resultMsg.rating = result.rating;
                } else {
                    resultMsg.reason = result.reason;
                }
                nlohmann::json resultPayload = resultMsg;
                sendTo(hdl, "login_result", resultPayload);

                if (!result.success) {
                    std::cout << "Login failed for email: " << login.username
                              << ", reason: " << result.reason << std::endl;
                    return;
                }

                std::string role;
                if (countLoggedIn() == 0) {
                    role = "white";
                } else if (countLoggedIn() == 1) {
                    role = "black";
                } else {
                    role = "spectator";
                }

                connectionInfos[hdl].loggedIn = true;
                connectionInfos[hdl].email = login.username;
                connectionInfos[hdl].role = role;

                protocol::RoomJoinedMessage joined{ROOM_ID, role};
                nlohmann::json joinedPayload = joined;
                sendTo(hdl, "room_joined", joinedPayload);

                std::cout << "Login succeeded for email: " << login.username
                          << ", assigned role: " << role << std::endl;
            } else if (type == "click") {
                auto it = connectionInfos.find(hdl);
                if (it == connectionInfos.end() || !it->second.loggedIn) {
                    std::cout << "Rejecting click from a connection that hasn't logged in yet." << std::endl;
                    protocol::ErrorMessage err{"You must log in before sending clicks."};
                    nlohmann::json errPayload = err;
                    sendTo(hdl, "error", errPayload);
                    return;
                }

                nlohmann::json parsed = nlohmann::json::parse(rawText);
                protocol::ClickMessage click = parsed.at("payload").get<protocol::ClickMessage>();

                Controller::click(state, click.x, click.y, colorFromRole(it->second.role));
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            }
        });

        game_server.listen(PORT);
        game_server.start_accept();

        std::cout << "game_server listening on ws://localhost:" << PORT << std::endl;

        auto previousTime = std::chrono::steady_clock::now();

        while (true) {
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

            for (const auto& entry : connectionInfos) {
                if (!entry.second.loggedIn) continue;

                websocketpp::lib::error_code ec;
                game_server.send(entry.first, rawText, websocketpp::frame::opcode::text, ec);
                if (ec) {
                    std::cerr << "Broadcast send failed: " << ec.message() << std::endl;
                }
            }

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
