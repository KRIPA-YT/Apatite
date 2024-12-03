#include "ModCmds.h"
#include "../Apatite.h"
#include "../api/User.h"

namespace cmds {
	ModCmds::ModCmds() {
		stopCommand = Command{
			.description = "Stops the bot!",
			.usage = "!stop",
			.handler = [](const CommandArgs& args) {
				if (!Apatite::fetchInstance().cmdManager.isModCmdAllowed(args.sender, args.broadcaster)) {
					Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("You don't have sufficient permissions to execute this command...");
					return true;
				}
				Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("Stopping...");
				Apatite::fetchInstance().stop();
				return true;
			}
		};
		addSudo = Command{
			.description = "Adds a user to sudoers!",
			.usage = "!addsudo <user>",
			.handler = [](const CommandArgs& args) {
				if (!Apatite::fetchInstance().cmdManager.isModCmdAllowed(args.sender, args.broadcaster)) {
					Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("You don't have sufficient permissions to execute this command...");
					return true;
				}
				if (!args.args.size() < 1) {
					return false;
				}
				Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("Someone tell Amy that she forgor to implement me!");
				return true;
			}
		};
	}

	void ModCmds::init() {
		Apatite::fetchInstance().cmdManager.hook("stop", stopCommand);
		Apatite::fetchInstance().cmdManager.hook("addsudo", addSudo);
		Apatite::fetchInstance().cmdManager.hook("modcmd", Command{
			.description = "",
			.usage = "",
			.handler = [](const CommandArgs& args) {
				if (!Apatite::fetchInstance().cmdManager.isModCmdAllowed(args.sender, args.broadcaster)) {
					Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("You don't have sufficient permissions to execute this command...");
					return true;
				}
				Apatite::fetchInstance().twitchAPIConnector.twitchChat.sendMessage("OMG Mod!");
				return true;
			}
		});
	}
}