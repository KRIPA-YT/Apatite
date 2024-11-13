#include "AuthConfig.h"
#include "../api/Tokens.h"
#include "../ManagedSingleton.h"

AuthConfig::AuthConfig() {

}

std::map<std::string, YAML::Node>* AuthConfig::getDefaults() {
	return new std::map<std::string, YAML::Node> {
		{"client_id", YAML::Load("")},
		{"client_secret", YAML::Load("")},
		{"access_token", YAML::Load("")},
		{"access_token_expiry", YAML::Load("0")},
		{"refresh_token", YAML::Load("")}
	};
}