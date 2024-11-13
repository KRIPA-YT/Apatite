#pragma once
#include "TwitchAPIAuthenticationServer.h"
#include "Tokens.h"
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <random>
#include <cmath> 

typedef websocketpp::client<websocketpp::config::asio_client> Client;

class TwitchAPIConnector {
public:
	void connect();
private:
	std::string clientID;
	Client client;
	void authenticate();
};