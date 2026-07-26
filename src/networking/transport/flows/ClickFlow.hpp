#pragma once

#include "../GameConnection.hpp"

class ClickFlow {
public:
    explicit ClickFlow(GameConnection& connection);

    bool isConnected() const;
    void sendClick(int x, int y);

private:
    GameConnection& connection_;
};
