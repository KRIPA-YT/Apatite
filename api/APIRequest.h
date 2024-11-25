#pragma once
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "TwitchAPIConnector.h"
#include "../Apatite.h"

using namespace cpr;
using json = nlohmann::json;

namespace twitch_api {
	enum AccessToken { APP, USER };
	enum RequestMethod { GET, POST };

	class Request {
	private:
		Session session;
		AccessToken accessToken = USER;
		RequestMethod requestMethod;
		std::map<uint16_t, std::function<json(Response)>*> handlers;
		int16_t successCode = 200; 

		void initHeader();
	public:
		Request();
		Request(std::string endpoint);
		Request(RequestMethod requestMethod, std::string endpoint);
		void setEndpoint(std::string endpoint);
		void setPayload(json payload);
		void setParameters(Parameters parameters);
		void setTokenType(AccessToken accessToken);
		void setRequestMethod(RequestMethod requestMethod);
		void addHandler(uint16_t responseCode, std::function<json(Response)>* handler);
		void setSuccessCode(uint16_t code);
		json request(TwitchAPIConnector& connector = *(Apatite::fetchInstance().twitchAPIConnector));
	};
}