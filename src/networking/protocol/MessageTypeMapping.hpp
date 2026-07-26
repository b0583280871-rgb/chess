#pragma once

#include <string>

#include "MessageTypes.hpp"

namespace protocol
{

    MessageType toMessageType(const std::string &name);

    std::string toString(MessageType type);

}
