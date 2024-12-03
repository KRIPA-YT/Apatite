#include "Chat.h"
#include "Tokens.h"
#include "../Apatite.h"
#include "Request.h"
#include <spdlog/spdlog.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
constexpr auto APATITE_TWITCH_UID = "1200898201";

namespace twitch {

    Chat::Chat() {

    }

    bool Chat::sendMessage(std::string message) {
        Request request = Request(POST, "chat/messages", Tokens::fetchInstance().botUserAccess);
        request.setPayload({
            {"broadcaster_id", "465362349"},
            {"sender_id", APATITE_TWITCH_UID},
            {"message", message}
            });
        request.addHandler(429, [&](cpr::Response response) {
            std::time_t ratelimitReset = std::stoi(response.header.find("Ratelimit-Reset")->second);
            std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(ratelimitReset));
            this->sendMessage(message);
            return json{};
            });
        request.request();
        spdlog::info("Sent message: {}", message);
        return true;
    }

    bool Chat::reply(std::string message, std::string messageId) {
        Request request = Request(POST, "chat/messages", Tokens::fetchInstance().botUserAccess);
        request.setPayload({
            {"broadcaster_id", "465362349"},
            {"sender_id", APATITE_TWITCH_UID},
            {"message", message},
            {"reply_parent_message_id", messageId}
            });
        request.addHandler(429, [&](cpr::Response response) {
            std::time_t ratelimitReset = std::stoi(response.header.find("Ratelimit-Reset")->second);
            std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(ratelimitReset));
            this->sendMessage(message);
            return json{};
            });
        request.request();
        spdlog::info("Sent reply to {}: {}", messageId, message);
        return true;
    }
}