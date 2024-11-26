#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <map>
#include <regex>
#include <sstream>
#include <iostream>
#include <random>
#include <cmath>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "Tokens.h"

namespace twitch {
	using boost::asio::ip::tcp;

	class AuthServer {
	public:
		TokenPair refreshUserAccessToken(std::string refresh);

		bool authenticateUser(TokenPair& tokenPair);;
		bool authenticateApp();
	private:
		std::string state;
		std::string code;

		bool validateToken(std::string token);

		void generateState(const std::vector<std::string> scopes);
		std::map<std::string, std::string> parseHttpHeaders(boost::core::basic_string_view<char>& request);
		bool handleRequest(boost::beast::http::request<boost::beast::http::string_body>& request, boost::asio::ip::tcp::socket& socket);
		void refererCatcher();
		TokenPair generateTokens();
		TokenPair firstAuth(const std::vector<std::string> scopes);
	};
}