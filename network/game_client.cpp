
#include "win32_ui/LoginWindow.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <opencv2/opencv.hpp>

#include <cctype>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include "SnapshotAdapter.hpp"
#include "protocol/JsonCodec.hpp"
#include "view/Renderer.hpp"
#include "audio/AudioPlayer.hpp"
#include "audio/GameEventDetector.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::connection_hdl connection_hdl;

namespace
{
    const std::string URI = "ws://localhost:9004";
    const std::string WINDOW_NAME = "Kung Fu Chess (networked)";

    std::string trim(const std::string &v)
    {
        size_t a = 0, b = v.size();
        while (a < b && std::isspace((unsigned char)v[a]))
            ++a;
        while (b > a && std::isspace((unsigned char)v[b - 1]))
            --b;
        return v.substr(a, b - a);
    }

    struct ClientContext
    {
        client *c = nullptr;
        connection_hdl hdl;
        bool connected = false;
    };

    void onMouse(int event, int x, int y, int /*flags*/, void *userdata)
    {
        if (event != cv::EVENT_LBUTTONDOWN)
            return;

        ClientContext *ctx = static_cast<ClientContext *>(userdata);
        if (!ctx->connected)
            return;

        protocol::ClickMessage click{x, y};
        nlohmann::json payload = click;
        nlohmann::json envelope = protocol::wrapEnvelope("click", payload);

        ctx->c->send(ctx->hdl, envelope.dump(), websocketpp::frame::opcode::text);
    }

    enum class FlowState
    {
        Idle,
        LoginRequested,
        RegisterRequested,
        WaitingForResponse,
    };
}

int main()
{
    client c;
    ClientContext ctx;
    ctx.c = &c;

    std::optional<GameSnapshot> latestSnapshot;
    bool hasNewSnapshot = false;

    bool loginResultReceived = false;
    bool loginSucceeded = false;
    std::string loginFailureReason;
    int loginRating = 0;

    bool registerResultReceived = false;
    bool registerSucceeded = false;
    std::string registerFailureReason;

    bool roomJoinedReceived = false;
    std::string myRole;
    std::string myRoomId;

    try
    {
        c.set_access_channels(websocketpp::log::alevel::none);
        c.clear_access_channels(websocketpp::log::alevel::all);

        c.init_asio();

        c.set_open_handler([&](connection_hdl hdl)
                           {
            ctx.hdl = hdl;
            ctx.connected = true;
            std::cout << "Connected to game_server." << std::endl; });

        c.set_message_handler([&](connection_hdl, client::message_ptr msg)
                              {
            const std::string rawText = msg->get_payload();

            std::string type;
            try {
                type = protocol::envelopeType(rawText);
            } catch (const std::exception& e) {
                std::cout << "Ignoring malformed message: " << e.what() << std::endl;
                return;
            }

            nlohmann::json parsed = nlohmann::json::parse(rawText);

            if (type == "login_result") {
                protocol::LoginResultMessage result = parsed.at("payload").get<protocol::LoginResultMessage>();
                loginResultReceived = true;
                loginSucceeded = result.success;
                if (result.success && result.rating.has_value()) {
                    loginRating = result.rating.value();
                } else if (!result.success && result.reason.has_value()) {
                    loginFailureReason = result.reason.value();
                }
            } else if (type == "register_result") {
                protocol::RegisterResultMessage result = parsed.at("payload").get<protocol::RegisterResultMessage>();
                registerResultReceived = true;
                registerSucceeded = result.success;
                registerFailureReason = result.reason;
            } else if (type == "room_joined") {
                protocol::RoomJoinedMessage joined = parsed.at("payload").get<protocol::RoomJoinedMessage>();
                roomJoinedReceived = true;
                myRole = joined.role;
                myRoomId = joined.room_id;
            } else if (type == "snapshot") {
                protocol::SnapshotMessage snapMsg = parsed.at("payload").get<protocol::SnapshotMessage>();
                latestSnapshot = SnapshotAdapter::fromProtocol(snapMsg);
                hasNewSnapshot = true;
            } else if (type == "error") {
                protocol::ErrorMessage err = parsed.at("payload").get<protocol::ErrorMessage>();
                std::cout << "Server error: " << err.message << std::endl;
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            } });

        c.set_fail_handler([](connection_hdl)
                           { std::cerr << "game_client: connection failed - is game_server running?" << std::endl; });

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(URI, ec);
        if (ec)
        {
            std::cerr << "game_client: could not create connection: " << ec.message() << std::endl;
            return 1;
        }

        c.connect(con);

        while (!ctx.connected)
        {
            c.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        win32_ui::LoginWindow window;
        if (!window.create())
        {
            std::cerr << "game_client: failed to create login window." << std::endl;
            return 1;
        }

        FlowState flowState = FlowState::Idle;
        bool waitingForLoginResponse = false;
        bool loggedIn = false;

        while (true)
        {
            if (!window.pumpMessages())
            {
                return 0;
            }
            c.poll();

            if (flowState == FlowState::Idle)
            {
                win32_ui::LoginAction action = window.takeRequestedAction();
                if (action == win32_ui::LoginAction::LoginRequested)
                {
                    flowState = FlowState::LoginRequested;
                }
                else if (action == win32_ui::LoginAction::RegisterRequested)
                {
                    flowState = FlowState::RegisterRequested;
                }
            }

            if (flowState == FlowState::LoginRequested)
            {
                std::string email = trim(window.email());
                std::string password = trim(window.password());

                loginResultReceived = false;
                protocol::LoginMessage login{email, password};
                nlohmann::json payload = login;
                nlohmann::json envelope = protocol::wrapEnvelope("login", payload);
                c.send(ctx.hdl, envelope.dump(), websocketpp::frame::opcode::text);

                window.setStatus("Logging in...");
                window.setEnabled(false);
                waitingForLoginResponse = true;
                flowState = FlowState::WaitingForResponse;
            }
            else if (flowState == FlowState::RegisterRequested)
            {
                std::string email = trim(window.email());
                std::string password = trim(window.password());

                registerResultReceived = false;
                protocol::RegisterMessage reg{email, password};
                nlohmann::json payload = reg;
                nlohmann::json envelope = protocol::wrapEnvelope("register", payload);
                c.send(ctx.hdl, envelope.dump(), websocketpp::frame::opcode::text);

                window.setStatus("Registering...");
                window.setEnabled(false);
                waitingForLoginResponse = false;
                flowState = FlowState::WaitingForResponse;
            }
            else if (flowState == FlowState::WaitingForResponse)
            {
                if (waitingForLoginResponse && loginResultReceived)
                {
                    if (loginSucceeded)
                    {
                        window.setStatus("Login succeeded!");
                        loggedIn = true;
                        break;
                    }
                    else
                    {
                        window.setStatus("Login failed: " + loginFailureReason);
                        window.setEnabled(true);
                        flowState = FlowState::Idle;
                    }
                }
                else if (!waitingForLoginResponse && registerResultReceived)
                {
                    if (registerSucceeded)
                    {
                        window.setStatus("Registered successfully - now log in.");
                    }
                    else
                    {
                        window.setStatus("Registration failed: " + registerFailureReason);
                    }
                    window.setEnabled(true);
                    flowState = FlowState::Idle;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        window.destroy();

        if (!loggedIn)
        {
            return 0;
        }

        std::cout << "Login succeeded. Rating: " << loginRating << std::endl;

        while (!roomJoinedReceived)
        {
            c.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        std::cout << "Assigned role: " << myRole << " (room " << myRoomId << ")" << std::endl;

        cv::namedWindow(WINDOW_NAME);
        cv::setMouseCallback(WINDOW_NAME, onMouse, static_cast<void *>(&ctx));

        audio::AudioPlayer audioPlayer;
        audio::GameEventDetector eventDetector;

        bool gameOverKeySeen = false;

        while (true)
        {
            c.poll();

            if (hasNewSnapshot && latestSnapshot)
            {
                audio::GameEventDetector::Events events = eventDetector.detect(*latestSnapshot);
                if (events.gameOverJustNow)
                {
                    audioPlayer.playGameOver();
                }
                else if (events.captureHappened)
                {
                    audioPlayer.playCapture();
                }
                else if (events.moveStarted)
                {
                    audioPlayer.playMove();
                }

                Img canvas = renderFrame(*latestSnapshot);
                cv::imshow(WINDOW_NAME, canvas.get_mat());
                hasNewSnapshot = false;
            }

            int key = cv::waitKey(1);

            if (key == 27 || key == 'q' || key == 'Q')
            {
                break;
            }

            if (latestSnapshot && latestSnapshot->gameOver)
            {
                if (gameOverKeySeen)
                    break;
                if (key != -1)
                    gameOverKeySeen = true;
            }
        }

        if (ctx.connected)
        {
            c.close(ctx.hdl, websocketpp::close::status::normal, "client exiting");
            c.poll();
        }

        cv::destroyAllWindows();
        return 0;
    }
    catch (websocketpp::exception const &e)
    {
        std::cerr << "game_client error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "game_client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
