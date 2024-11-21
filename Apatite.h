#pragma once

#include <iostream>
#include <random>
#include <cmath>
#include <spdlog/spdlog.h>
#include "ManagedSingleton.h"
#include "api/TwitchAPIConnector.h"
#include "config/AuthConfig.h"
#include "cmds/CmdManager.h"

class Apatite {
public:
	Apatite();
	~Apatite();
	TwitchAPIConnector* twitchAPIConnector;
	AuthConfig* authConfig;
	CmdManager* cmdManager;

	void restart();

	static Apatite& fetchInstance();
private:
	void run();
};