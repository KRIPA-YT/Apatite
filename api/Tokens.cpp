#include "Tokens.h"
#include "../Apatite.h"

Tokens::Tokens() {
	YAML::Node* config = &Apatite::fetchInstance().authConfig->config;
	access = (*config)["access_token"].as<std::string>();
	clientId = (*config)["client_id"].as<std::string>();
	clientSecret = (*config)["client_secret"].as<std::string>();
	refresh = (*config)["refresh_token"].as<std::string>();
	accessExpiry = std::stoi((*config)["access_token_expiry"].as<std::string>());
}

Tokens::~Tokens() {
	YAML::Node* config = &Apatite::fetchInstance().authConfig->config;
	(*config)["access_token"] = YAML::Load(access);
	(*config)["client_id"] = YAML::Load(clientId);
	(*config)["client_secret"] = YAML::Load(clientSecret);
	(*config)["refresh_token"] = YAML::Load(refresh);
	(*config)["access_token_expiry"] = YAML::Load(std::to_string(accessExpiry));
	Apatite::fetchInstance().authConfig->save();
}

Tokens& Tokens::fetchInstance() {
	if (ManagedSingleton<Tokens>::instance() == nullptr) {
		ManagedSingleton<Tokens>::createInstance();
	}
	return *(ManagedSingleton<Tokens>::instance());
}