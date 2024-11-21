#include "TwitchChat.h"
#include "Tokens.h"
#include "../Apatite.h"
#include <spdlog/spdlog.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
constexpr auto PREFIX = "[Apatite] ";
constexpr auto APATITE_TWITCH_UID = "1200898201";

bool TwitchChat::sendMessage(std::string message) {
    json body = {
        {"broadcaster_id", "465362349"},
        {"sender_id", APATITE_TWITCH_UID},
        {"message", message}
    };
    cpr::Response response = cpr::Post(
        cpr::Url{"https://api.twitch.tv/helix/chat/messages"},
        cpr::Header{
            {"Authorization", "Bearer " + Tokens::fetchInstance().userAccess},
            {"Client-Id", Tokens::fetchInstance().clientId},
            {"Content-Type", "application/json"}
        },
        cpr::Body{body.dump()}
    );
    if (response.status_code == 401) { // Unauthorized
        if (!Apatite::fetchInstance().twitchAPIConnector->authenticate()) return false;
        return this->sendMessage(message);
    }
    if (response.status_code != 200) { // Some other error
        spdlog::error("Couldn't send message!");
        spdlog::debug("HTTP Status Code: {}", response.status_code);
        spdlog::debug("HTTP response: {}", response.text);
    }
    spdlog::info("Sent message: {}", message);
    return true;
}

bool TwitchChat::sendPlainMessage(std::string message) {
	return this->sendMessage(PREFIX + message);
}