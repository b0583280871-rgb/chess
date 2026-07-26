#pragma once

#include <string>

#include "MessageTypes.hpp"

namespace protocol
{

    // Throws std::runtime_error if name isn't a recognized message type.
    MessageType toMessageType(const std::string &name);

    std::string toString(MessageType type);

}
