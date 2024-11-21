#include "Tokens.h"
#include "../Apatite.h"

Tokens::Tokens() {
	YAML::Node* config = &Apatite::fetchInstance().authConfig->config;
	userAccess = (*config)["user_access_token"].as<std::string>();
	appAccess = (*config)["app_access_token"].as<std::string>();
	clientId = (*config)["client_id"].as<std::string>();
	clientSecret = (*config)["client_secret"].as<std::string>();
	refresh = (*config)["refresh_token"].as<std::string>();
	try {
		userAccessExpiry = std::stoi((*config)["user_access_token_expiry"].as<std::string>());
	} catch (...) {
		userAccessExpiry = 0;
	}
	try {
		appAccessExpiry = std::stoi((*config)["app_access_token_expiry"].as<std::string>());
	} catch (...) {
		appAccessExpiry = 0;
	}
}

Tokens::~Tokens() {
	spdlog::info("Saving auth.yml...");
	YAML::Node* config = &Apatite::fetchInstance().authConfig->config;

	(*config)["user_access_token"] = YAML::Load(userAccess);
	(*config)["user_access_token_expiry"] = YAML::Load(std::to_string(userAccessExpiry));
	(*config)["app_access_token"] = YAML::Load(appAccess);
	(*config)["app_access_token_expiry"] = YAML::Load(std::to_string(appAccessExpiry));
	(*config)["client_id"] = YAML::Load(clientId);
	(*config)["client_secret"] = YAML::Load(clientSecret);
	(*config)["refresh_token"] = YAML::Load(refresh);
	Apatite::fetchInstance().authConfig->save();
}

Tokens& Tokens::fetchInstance() {
	if (ManagedSingleton<Tokens>::instance() == nullptr) {
		ManagedSingleton<Tokens>::createInstance();
	}
	return *(ManagedSingleton<Tokens>::instance());
}