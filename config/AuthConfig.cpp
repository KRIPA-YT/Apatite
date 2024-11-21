#include "AuthConfig.h"
#include "../api/Tokens.h"
#include "../ManagedSingleton.h"

AuthConfig::AuthConfig() {

}

std::map<std::string, YAML::Node>* AuthConfig::getDefaults() {
	return new std::map<std::string, YAML::Node> {
		{"client_id", YAML::Load("")},
		{"client_secret", YAML::Load("")},
		{"app_access_token", YAML::Load("")},
		{"app_access_token_expiry", YAML::Load("0")},
		{"user_access_token", YAML::Load("")},
		{"user_access_token_expiry", YAML::Load("0")},
		{"refresh_token", YAML::Load("")}
	};
}