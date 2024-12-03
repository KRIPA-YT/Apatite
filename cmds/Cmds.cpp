#include "Cmds.h"
#include "../Apatite.h"
#include "../api/User.h"
#include <spdlog/spdlog.h>

namespace cmds {
	Cmds::Cmds() {
		sudoCommand = Command{
			.description = "Priviledge escalation!",
			.usage = "!sudo <command>",
			.handler = [](const CommandArgs& args) {
				if (args.args.size() < 1) {
					return false;
				}

				spdlog::debug("FIRING");
				spdlog::debug("args.broadcaster.sudoers.size() = {}", args.broadcaster.sudoers.size());
				if (!args.sender.isSudoer(args.broadcaster)) {
					Apatite::fetchInstance().twitchAPIConnector.twitchChat.reply(std::format("\'{}\' is not in the sudoers file. This incident will be reported.", args.sender.name), args.messageId);
					return true;
				}
				CommandArgs cmdArgs = args;
				std::string label = cmdArgs.args[0];
				cmdArgs.args.erase(cmdArgs.args.begin());
				Apatite::fetchInstance().cmdManager.elevFire(label, cmdArgs);
				return true;
			}
		};
	}

	void Cmds::init() {
		Apatite::fetchInstance().cmdManager.hook("sudo", sudoCommand);
	}
}