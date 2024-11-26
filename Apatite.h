#pragma once

#include <iostream>
#include <random>
#include <cmath>
#include <spdlog/spdlog.h>
#include "ManagedSingleton.h"
#include "api/TwitchAPIConnector.h"
#include "config/AuthConfig.h"
#include "cmds/CmdManager.h"
#include "cmds/ModCmds.h"

class Apatite {
public:
	~Apatite();
	TwitchAPIConnector twitchAPIConnector;
	AuthConfig authConfig;
	CmdManager cmdManager;
	ModCmds modCmds;

	void restart();
	void stop();

	static Apatite& fetchInstance();
private:
	bool running;
	void run();
};