#include "TwitchAPIConnector.h"
#include "../Apatite.h"
#include <spdlog/spdlog.h>
#include "APIRequest.h"

constexpr auto WEBSOCKET_HOSTNAME = "eventsub.wss.twitch.tv/ws";
constexpr auto WEBSOCKET_URI = "wss://eventsub.wss.twitch.tv/ws";
constexpr auto TWITCH_API_URL = "https://api.twitch.tv/helix/";
constexpr auto APATITE_TWITCH_UID = "1200898201";
namespace placeholders = websocketpp::lib::placeholders;

using namespace twitch;

TwitchAPIConnector::TwitchAPIConnector() {
	this->authServer = TwitchAPIAuthenticationServer();
    this->twitchChat = new TwitchChat();
}

TwitchAPIConnector::~TwitchAPIConnector() {
	delete this->twitchChat;
}

bool TwitchAPIConnector::authenticate() {
    if (!this->authServer.authenticateApp()) return false;
    Tokens::fetchInstance().botUserAccess.scopes = {
        "channel:bot",
        "chat:read",
        "user:read:chat",
        "user:write:chat",
        "user:bot",
        "chat:edit"
    };
    return this->authServer.authenticateUser(Tokens::fetchInstance().botUserAccess);
}

void TwitchAPIConnector::hook(std::string event, std::function<void(json)> handler) {
    this->notificationHandlers.insert({event, handler});
}

void TwitchAPIConnector::unhook(std::string event) {
    this->notificationHandlers.erase(event);
}

bool TwitchAPIConnector::connect() {
	if (!this->authenticate()) return false; // Return false when fails
    if (!this->connectWebsocket()) return false; // Return false when fails
	return true;
}

context_ptr TwitchAPIConnector::on_tls_init(const char* hostname, websocketpp::connection_hdl) {
    context_ptr context = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    try {
        context->set_options(boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::single_dh_use);
    }
    catch (std::exception& exc) {
        spdlog::error("TLS Initialization Error: {}", exc.what());
    }

    return context;
}

void TwitchAPIConnector::on_open(websocketpp::connection_hdl hdl, Client* c) {
    spdlog::debug("Websocket connected!");
    websocketpp::lib::error_code errorCode;
    Client::connection_ptr connection = c->get_con_from_hdl(hdl, errorCode);

    if (errorCode) {
        spdlog::error("Failed to get connection pointer: {}", errorCode.message());
        return;
    }
}

void TwitchAPIConnector::on_message(websocketpp::connection_hdl hdl, Client::message_ptr msg) {
    spdlog::debug("Received message: {}", msg->get_payload());
    json message = json::parse(msg->get_payload());
    if (message["metadata"]["message_type"].get<std::string>() == "session_welcome") {
        this->handleSessionWelcome(message, hdl, msg);
        return;
    }
    if (message["metadata"]["message_type"].get<std::string>() == "session_keepalive") {
        // TODO: Keepalive
        return;
    }
    if (message["metadata"]["message_type"].get<std::string>() == "notification") {
        this->handleNotification(message);
        return;
    }
}

void TwitchAPIConnector::handleNotification(json& message) {
    for (auto it = this->notificationHandlers.begin(); it != this->notificationHandlers.end(); it++) {
        if (it->first != message["metadata"]["subscription_type"].get<std::string>()) {
            spdlog::debug("sub_type={}, it->first={}", message["metadata"]["subscription_type"].get<std::string>(), it->first);
            continue;
        }
        std::thread(std::bind(it->second, message["payload"]["event"])).detach(); // TODO: Join mechanism
    }
}

void TwitchAPIConnector::handleSessionWelcome(json& message, websocketpp::connection_hdl& hdl, std::shared_ptr<websocketpp::config::core_client::message_type>& msg) {
    this->sessionID = message["payload"]["session"]["id"].get<std::string>();
    if (this->subscribe()) {
        return;
    }
    // If fails, reauthenticate
    if (!this->authenticate()) {
        spdlog::error("Can't reauthenticate!");
        return;
    }
    this->on_message(hdl, msg);
    return;
}

void TwitchAPIConnector::on_fail(websocketpp::connection_hdl hdl) {
    spdlog::error("Websocket connection failed!");
}

void TwitchAPIConnector::on_close(websocketpp::connection_hdl hdl) {
    spdlog::info("Websocket connection closed!");
    Apatite::fetchInstance().stop();
}

bool TwitchAPIConnector::connectWebsocket() {
    spdlog::info("Connecting to Twitch API...");
    try {
        client.set_access_channels(websocketpp::log::alevel::all);
        client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        client.set_error_channels(websocketpp::log::elevel::all);
        client.init_asio();

        client.set_message_handler(bind(&TwitchAPIConnector::on_message, this, placeholders::_1, placeholders::_2));
        client.set_tls_init_handler(bind(&TwitchAPIConnector::on_tls_init, this, WEBSOCKET_HOSTNAME, placeholders::_1));

        client.set_open_handler(bind(&TwitchAPIConnector::on_open, this, placeholders::_1, &client));
        client.set_fail_handler(bind(&TwitchAPIConnector::on_fail, this, placeholders::_1));
        client.set_close_handler(bind(&TwitchAPIConnector::on_close, this, placeholders::_1));
        client.set_error_channels(websocketpp::log::elevel::all);
        websocketpp::lib::error_code errorCode;
        Client::connection_ptr connection = client.get_connection(WEBSOCKET_URI, errorCode);
        if (errorCode) {
            std::cout << "Could not create connection because: " << errorCode.message() << std::endl;
            return false;
        }
        client.connect(connection);

        return true;
    }
    catch (websocketpp::exception const& exc) {
        spdlog::error("Exception while connecting to Twitch API: {}", exc.what());
        return false;
    }
}

void TwitchAPIConnector::run() {
    client.run_one();
}

bool TwitchAPIConnector::subscribe() {
    Request request = Request(POST, "eventsub/subscriptions", Tokens::fetchInstance().botUserAccess);
    request.setPayload({
        {"type", "channel.chat.message"},
        {"version", "1"},
        {"condition", {
            {"broadcaster_user_id", "465362349"}, // TODO: Move to config
            {"user_id", APATITE_TWITCH_UID}
        }},
        {"transport", {
            {"method", "websocket"},
            {"session_id", this->sessionID}
        }}
    });
    request.setSuccessCode(202);
    json response = request.request();
    spdlog::debug("Subscription response: {}", response.dump());
    return true;
}