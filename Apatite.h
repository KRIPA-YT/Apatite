#pragma once

#include <iostream>
#include <random>
#include <cmath>
#include "ManagedSingleton.h"
#include "api/TwitchAPIConnector.h"
#include "config/AuthConfig.h"

// Fix the circular inclusion so we can actually use this

class Apatite {
public:
	Apatite();
	~Apatite();
	TwitchAPIConnector* twitchAPIConnector;
	AuthConfig* authConfig;

	void restart();

	static Apatite& fetchInstance();
private:
};