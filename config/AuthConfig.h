#pragma once
#include "DefaultConfig.h"

class AuthConfig : public DefaultConfig {
public:
	AuthConfig();

	~AuthConfig() {

	}
protected:
	std::map<std::string, YAML::Node>* getDefaults() override;
};