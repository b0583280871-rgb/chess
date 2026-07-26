#include "ClickFlow.hpp"

#include "networking/protocol/JsonCodec.hpp"

ClickFlow::ClickFlow(GameConnection& connection) : connection_(connection) {
}

bool ClickFlow::isConnected() const {
    return connection_.isConnected();
}

void ClickFlow::sendClick(int x, int y) {
    protocol::ClickMessage click{x, y};
    connection_.sendMessage(click);
}
