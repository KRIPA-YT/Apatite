#pragma once
#include "AuthServer.h"
#include "Tokens.h"
#include "Chat.h"
#include "../cmds/CmdManager.h"
#include <websocketpp/config/asio_client.hpp>
#include <boost/beast/ssl.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <functional>

namespace twitch {
	typedef websocketpp::client<websocketpp::config::asio_tls_client> Client;
	typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
	using json = nlohmann::json;
	using NotificationHandlerMap = std::map<std::string, std::function<void(json)>>;

	class Connector {
	public:
		Chat twitchChat;

		bool connect();
		bool authenticate();
		void run();

		void hook(std::string, std::function<void(json)>);
		void unhook(std::string);
	private:
		std::string clientID;
		std::string sessionID;
		NotificationHandlerMap notificationHandlers = NotificationHandlerMap();
		AuthServer authServer;
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
}