#include "AuthConfig.h"
#include "../api/Tokens.h"
#include "../ManagedSingleton.h"

namespace config {
	AuthConfig::AuthConfig() {

	}

	std::map<std::string, YAML::Node>* AuthConfig::getDefaults() {
		return new std::map<std::string, YAML::Node>{
			{"client_id", YAML::Load("")},
			{"client_secret", YAML::Load("")},
			{"app_access_token", YAML::Load("")},
			{"user_access_token", YAML::Load("")},
			{"refresh_token", YAML::Load("")}
		};
	}
}