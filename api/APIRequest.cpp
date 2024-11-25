#include "APIRequest.h"
#include <spdlog\spdlog.h>

constexpr auto TWITCH_API_URL = "https://api.twitch.tv/helix/";
namespace twitch_api {
	Request::Request() {
		this->initHeader();
	}

	Request::Request(std::string endpoint) {
		this->setEndpoint(endpoint);
		this->initHeader();
	}

	Request::Request(RequestMethod requestMethod, std::string endpoint) {
		this->setEndpoint(endpoint);
		this->setRequestMethod(requestMethod);
		this->initHeader();
	}

	void Request::initHeader() {
		this->session.SetOption(cpr::Header{
			{"Client-Id", Tokens::fetchInstance().clientId},
			{"Content-Type", "application/json"}
		});
	}

	void Request::setEndpoint(std::string endpoint) {
		this->session.SetOption(Url(TWITCH_API_URL + endpoint));
	}

	void Request::setPayload(json payload) {
		this->session.SetOption(Body(payload.dump()));
	}

	void Request::setParameters(Parameters params) {
		this->session.SetOption(params);
	}

	void Request::setSuccessCode(uint16_t successCode) {
		this->successCode = successCode;
	}

	void Request::setTokenType(AccessToken accessToken) {
		this->accessToken = accessToken;
	}

	void Request::setRequestMethod(RequestMethod requestMethod) {
		this->requestMethod = requestMethod;
	}

	void Request::addHandler(uint16_t responseCode, std::function<json(Response)>* handler) {
		this->handlers.insert({ responseCode, handler});
	}

	json Request::request(TwitchAPIConnector& connector) {
		session.SetOption(cpr::Bearer(this->accessToken == USER ? Tokens::fetchInstance().userAccess : Tokens::fetchInstance().appAccess));

		// TODO: Check if anything is set
		auto response = this->requestMethod == GET ? session.Get() : session.Post();
		if (this->handlers.find(response.status_code) != this->handlers.end()) {
			return (*this->handlers[response.status_code])(response);
		}
		if (response.status_code == 401) {
			spdlog::info("Unauthorized, trying to reauthorize...");
			spdlog::debug("Error: {}", response.text);
			if (!connector.authenticate()) return {};
			return this->request(connector); // Retry
		}
		if (response.status_code != this->successCode) {
			spdlog::error("API Request failed!");
			spdlog::debug("Status Code: {} {} {}", response.status_code, response.reason, response.error.message);
			spdlog::debug("API Request - Method: {}, URL: {}", requestMethod == GET ? "GET" : "POST", session.GetFullRequestUrl());
			return {};
		}
		return json::parse(response.text);
	}
}

