
#include "client-ui/LoginWindow.hpp"

#include "networking/transport/GameConnection.hpp"
#include "networking/transport/AuthFlow.hpp"

#include <opencv2/opencv.hpp>

#include <cctype>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include "networking/adapters/SnapshotAdapter.hpp"
#include "networking/protocol/JsonCodec.hpp"
#include "view/Renderer.hpp"
#include "audio/AudioPlayer.hpp"
#include "audio/GameEventDetector.hpp"

namespace {
    const std::string URI = "ws://localhost:9004";
    const std::string WINDOW_NAME = "Kung Fu Chess (networked)";


    std::string trim(const std::string& v) {
        size_t a = 0, b = v.size();
        while (a < b && std::isspace((unsigned char)v[a])) ++a;
        while (b > a && std::isspace((unsigned char)v[b - 1])) --b;
        return v.substr(a, b - a);
    }


    void onMouse(int event, int x, int y, int /*flags*/, void* userdata) {
        if (event != cv::EVENT_LBUTTONDOWN) return;

        GameConnection* connection = static_cast<GameConnection*>(userdata);
        if (!connection->isConnected()) return;

        connection->sendClick(x, y);
    }
}

int main() {
    GameConnection connection(URI);
    AuthFlow authFlow(connection);

    std::optional<GameSnapshot> latestSnapshot;
    bool hasNewSnapshot = false;

    bool roomJoinedReceived = false;
    std::string myRole;
    std::string myRoomId;

    try {
        connection.setMessageHandler([&](const std::string& type, const nlohmann::json& payload) {
            if (type == "login_result" || type == "register_result") {
                authFlow.handleMessage(type, payload);
            } else if (type == "room_joined") {
                protocol::RoomJoinedMessage joined = payload.get<protocol::RoomJoinedMessage>();
                roomJoinedReceived = true;
                myRole = joined.role;
                myRoomId = joined.room_id;
            } else if (type == "snapshot") {
                protocol::SnapshotMessage snapMsg = payload.get<protocol::SnapshotMessage>();
                latestSnapshot = SnapshotAdapter::fromProtocol(snapMsg);
                hasNewSnapshot = true;
            } else if (type == "error") {
                protocol::ErrorMessage err = payload.get<protocol::ErrorMessage>();
                std::cout << "Server error: " << err.message << std::endl;
            } else {
                std::cout << "unhandled type: " << type << std::endl;
            }
        });

        if (!connection.connect()) {
            return 1;
        }

        while (!connection.isConnected()) {
            connection.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        win32_ui::LoginWindow window;
        if (!window.create()) {
            std::cerr << "game_client: failed to create login window." << std::endl;
            return 1;
        }

        bool loggedIn = false;

        while (true) {
            if (!window.pumpMessages()) {
                return 0;
            }
            connection.poll();

            if (authFlow.state() == FlowState::Idle) {
                win32_ui::LoginAction action = window.takeRequestedAction();
                if (action == win32_ui::LoginAction::LoginRequested) {
                    std::string email = trim(window.email());
                    std::string password = trim(window.password());

                    authFlow.requestLogin(email, password);

                    window.setStatus("Logging in...");
                    window.setEnabled(false);
                } else if (action == win32_ui::LoginAction::RegisterRequested) {
                    std::string email = trim(window.email());
                    std::string password = trim(window.password());

                    authFlow.requestRegister(email, password);

                    window.setStatus("Registering...");
                    window.setEnabled(false);
                }
            } else if (authFlow.state() == FlowState::WaitingForResponse) {
                if (authFlow.loginResultReceived()) {
                    if (authFlow.loginSucceeded()) {
                        window.setStatus("Login succeeded!");
                        loggedIn = true;
                        break;
                    } else {
                        window.setStatus("Login failed: " + authFlow.loginFailureReason());
                        window.setEnabled(true);
                        authFlow.acknowledgeResult();
                    }
                } else if (authFlow.registerResultReceived()) {
                    if (authFlow.registerSucceeded()) {
                        window.setStatus("Registered successfully - now log in.");
                    } else {
                        window.setStatus("Registration failed: " + authFlow.registerFailureReason());
                    }
                    window.setEnabled(true);
                    authFlow.acknowledgeResult();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        window.destroy();

        if (!loggedIn) {
            return 0;
        }

        std::cout << "Login succeeded. Rating: " << authFlow.loginRating() << std::endl;

        while (!roomJoinedReceived) {
            connection.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        std::cout << "Assigned role: " << myRole << " (room " << myRoomId << ")" << std::endl;

        cv::namedWindow(WINDOW_NAME);
        cv::setMouseCallback(WINDOW_NAME, onMouse, static_cast<void*>(&connection));

        audio::AudioPlayer audioPlayer;
        audio::GameEventDetector eventDetector;

        bool gameOverKeySeen = false;

        while (true) {
            connection.poll();

            if (hasNewSnapshot && latestSnapshot) {
                audio::GameEventDetector::Events events = eventDetector.detect(*latestSnapshot);
                if (events.gameOverJustNow) {
                    audioPlayer.playGameOver();
                } else if (events.captureHappened) {
                    audioPlayer.playCapture();
                } else if (events.moveStarted) {
                    audioPlayer.playMove();
                }

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

        connection.close();

        cv::destroyAllWindows();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "game_client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
