#pragma once
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "TwitchAPIConnector.h"
#include "../Apatite.h"

using namespace cpr;
using json = nlohmann::json;

namespace twitch {
	enum RequestMethod { GET, POST };

	class Request {
	private:
		Session session;
		RequestMethod requestMethod;
		TokenPair& accessTokens;
		std::map<uint16_t, std::function<json(Response)>> handlers;
		int16_t successCode = 200; 

		void initHeader();
	public:
		Request(TokenPair& accessTokens);
		Request(std::string endpoint, TokenPair& accessTokens);
		Request(RequestMethod requestMethod, std::string endpoint, TokenPair& accessTokens);
		void setEndpoint(std::string endpoint);
		void setPayload(json payload);
		void setParameters(Parameters parameters);
		void setAccessTokens(TokenPair accessTokens);
		void setRequestMethod(RequestMethod requestMethod);
		void addHandler(uint16_t responseCode, std::function<json(Response)> handler);
		void setSuccessCode(uint16_t code);
		json request(TwitchAPIConnector& connector = *(Apatite::fetchInstance().twitchAPIConnector));
	};
}