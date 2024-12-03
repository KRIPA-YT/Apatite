#pragma once

#include "api/APIConnector.h"
#include "config/AuthConfig.h"
#include "cmds/CmdManager.h"
#include "cmds/ModCmds.h"
#include "cmds/Cmds.h"
#include "sql/SQLConnector.h"

class Apatite {
public:
	Apatite();
	~Apatite();
	twitch::Connector twitchAPIConnector;
	config::AuthConfig authConfig;
	cmds::CmdManager cmdManager;
	cmds::ModCmds modCmds;
	cmds::Cmds cmds;
	sql::Connector sqlConnector;

	void restart();
	void stop();

	static Apatite& fetchInstance();
private:
	bool running;
	void run();
};