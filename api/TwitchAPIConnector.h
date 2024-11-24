#pragma once
#include "TwitchAPIAuthenticationServer.h"
#include "Tokens.h"
#include "TwitchChat.h"
#include "../cmds/CmdManager.h"
#include <websocketpp/config/asio_client.hpp>
#include <boost/beast/ssl.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <functional>

typedef websocketpp::client<websocketpp::config::asio_tls_client> Client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
using json = nlohmann::json;
using NotificationHandlerMap = std::map<std::string, std::function<void(json)>>;

enum RequestMethod {
	POST,
	GET
};

class TwitchAPIConnector {
public:
	TwitchAPIConnector();
	~TwitchAPIConnector();
	TwitchChat* twitchChat;

	bool connect();
	bool authenticate();
	void run();
	json apiRequest(RequestMethod requestMethod, std::string url, json payload, uint16_t success);

	void hook(std::string, std::function<void(json)>);
	void unhook(std::string);
private:
	std::string clientID;
	std::string sessionID;
	NotificationHandlerMap notificationHandlers = NotificationHandlerMap();
	TwitchAPIAuthenticationServer* authServer;
	Client client;

	context_ptr on_tls_init(const char* hostname, websocketpp::connection_hdl);
	void on_message(websocketpp::connection_hdl, Client::message_ptr msg);
	void handleNotification(json& message);
	void handleSessionWelcome(json& message, websocketpp::connection_hdl& hdl, std::shared_ptr<websocketpp::config::core_client::message_type>& msg);
	void on_open(websocketpp::connection_hdl hdl, Client* c);
	void on_fail(websocketpp::connection_hdl hdl);
	void on_close(websocketpp::connection_hdl hdl);
	bool connectWebsocket();
	bool subscribe();
};