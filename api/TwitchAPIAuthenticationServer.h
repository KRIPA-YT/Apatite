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


using boost::asio::ip::tcp;

class TwitchAPIAuthenticationServer {
public:
	bool refreshAccessToken();
	void authenticate();
private:
	std::string state;
	std::string code;

	void generateState();
	std::map<std::string, std::string> parseHttpHeaders(boost::core::basic_string_view<char>& request);
	bool handleRequest(boost::beast::http::request<boost::beast::http::string_body>& request, boost::asio::ip::tcp::socket& socket);
	void refererCatcher();
	bool generateTokens();

	void firstAuth();

	bool isTokensValid();
	bool isAccessTokenExpired();
};