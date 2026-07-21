#include "JsonCodec.hpp"

#include <stdexcept>

namespace protocol
{

    nlohmann::json wrapEnvelope(const std::string &typeName, const nlohmann::json &payload)
    {
        return nlohmann::json{{"type", typeName}, {"payload", payload}};
    }

    std::string envelopeType(const std::string &rawJsonText)
    {
        nlohmann::json parsed;
        try
        {
            parsed = nlohmann::json::parse(rawJsonText);
        }
        catch (const nlohmann::json::parse_error &e)
        {
            throw std::runtime_error(std::string("envelopeType: invalid JSON: ") + e.what());
        }

        if (!parsed.contains("type") || !parsed.at("type").is_string())
        {
            throw std::runtime_error("envelopeType: missing or non-string 'type' field");
        }

        return parsed.at("type").get<std::string>();
    }

}
