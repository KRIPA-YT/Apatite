#include "Tokens.h"
#include "../Apatite.h"


namespace twitch {
	Tokens::Tokens() {
		YAML::Node& config = Apatite::fetchInstance().authConfig.config;
		appAccess = config["app_access_token"].as<std::string>();
		botUserAccess = TokenPair{
			.access = config["user_access_token"].as<std::string>(),
			.refresh = config["refresh_token"].as<std::string>()
		};
		clientId = config["client_id"].as<std::string>();
		clientSecret = config["client_secret"].as<std::string>();

		for (auto it = config.begin(); it != config.end(); it++) {
			try {
				std::string userIdString = it->first.as<std::string>();
				uint64_t userId = std::stoi(userIdString);
				userTokenPairs[userId] = TokenPair{
					.access = config[userIdString]["user_access_token"].as<std::string>(),
					.refresh = config[userIdString]["refresh_token"].as<std::string>()
				};
			} catch (std::invalid_argument) {} // Filtering out all the non user-specific tokens
		}
	}

	Tokens::~Tokens() {
		this->save();
	}

	void Tokens::save() {
		spdlog::info("Saving auth.yml...");
		YAML::Node& config = Apatite::fetchInstance().authConfig.config;

		config["app_access_token"] = YAML::Load(appAccess);
		config["user_access_token"] = YAML::Load(botUserAccess.access);
		config["refresh_token"] = YAML::Load(botUserAccess.refresh);
		config["client_id"] = YAML::Load(clientId);
		config["client_secret"] = YAML::Load(clientSecret);

		for (auto const& [id, tokenPair] : this->userTokenPairs) {
			std::string userId = std::to_string(id);
			config[userId]["user_access_token"] = tokenPair.access;
			config[userId]["refresh_token"] = tokenPair.refresh;
		}
		Apatite::fetchInstance().authConfig.save();
	}

	TokenPair& Tokens::getUserAccess(uint64_t userId) {
		YAML::Node& config = Apatite::fetchInstance().authConfig.config;
		if (this->userTokenPairs.find(userId) != this->userTokenPairs.end()) {
			return this->userTokenPairs[userId];
		}
		AuthServer authServer;
		this->userTokenPairs[userId] = TokenPair{
			.scopes = {"moderation:read"}
		};
		if (!authServer.authenticateUser(this->userTokenPairs[userId])) throw std::exception("Couldn't authenticate!");
		return this->userTokenPairs[userId];
	}

	Tokens& Tokens::fetchInstance() {
		if (ManagedSingleton<Tokens>::instance() == nullptr) {
			ManagedSingleton<Tokens>::createInstance();
		}
		return *(ManagedSingleton<Tokens>::instance());
	}
}