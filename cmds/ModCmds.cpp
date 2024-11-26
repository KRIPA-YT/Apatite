#include "ModCmds.h"
#include "../Apatite.h"
#include "../api/User.h"

ModCmds::ModCmds() {
	stopCommand = Command{
		.description = "Stops the bot!",
		.usage = "!stop",
		.handler = [](CommandArgs args) {
			if (!args.sender.isMod(args.broadcaster)) {
				Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("You don't have sufficient permissions to execute this command...");
				return true;
			}
			Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("Stopping...");
			Apatite::fetchInstance().stop();
			return true;
		}
	};
}

void ModCmds::init() {
	Apatite::fetchInstance().cmdManager.hook("stop", stopCommand);
}