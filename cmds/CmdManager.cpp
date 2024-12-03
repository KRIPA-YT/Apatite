#include "CmdManager.h"
#include "../Apatite.h"
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>

namespace cmds {
	constexpr char PREFIX = '!'; // TODO: Move to config

	void CmdManager::hookSubscription() {
		Apatite::fetchInstance().twitchAPIConnector.hook("channel.chat.message", std::bind(&CmdManager::onMessage, this, std::placeholders::_1));
	}

	void CmdManager::hook(std::string label, Command command) {
		this->cmdHandlerMap.insert({label, command});
	}

	void CmdManager::unhook(std::string label) {
		this->cmdHandlerMap.erase(label);
	}

	void CmdManager::fire(std::string label, const CommandArgs& args) {
		spdlog::debug("Fired command !{}, with args:", label);
		for (std::string arg : args.args) {
			spdlog::debug(arg);
		}
		if (this->cmdHandlerMap.find(label) == this->cmdHandlerMap.end()) {
			
			// TODO: Command not found
			return;
		}
		try {
			if (this->cmdHandlerMap[label].handler(args)) {
				return;
			}
		} catch (const std::exception& exc) {
			spdlog::error("Exception thrown while executing command !{}", label);
			spdlog::error(exc.what());
		}
		// TODO: Print usage message
	}

	void CmdManager::elevFire(std::string label, const CommandArgs& args) {
		spdlog::debug("Elevated firing!");
		this->currentSudoers.push_back(args.sender.id);
		this->fire(label, args);
		this->currentSudoers.pop_back();
	}

	bool CmdManager::isModCmdAllowed(const twitch::User& user, const twitch::Broadcaster& broadcaster) {
		return user.isMod(broadcaster) || std::find(this->currentSudoers.begin(), this->currentSudoers.end(), user.id) != this->currentSudoers.end();
	}

	void CmdManager::onMessage(const json& json) {
		std::string message = boost::trim_copy(json["message"]["text"].get<std::string>());
		if (!message.starts_with(PREFIX)) {
			return;
		}

		std::vector<std::string> command;
		boost::split(command, message, boost::is_any_of("\t, "));
		std::string label = command[0].substr(1);
		command.erase(command.begin());
		std::vector<std::string> args = command;

		CommandArgs cmdArgs = {
			.args = args,
			.broadcaster = twitch::Broadcaster(
				std::stoi(json["broadcaster_user_id"].get<std::string>()),
				json["broadcaster_user_name"].get<std::string>(),
				json["broadcaster_user_login"].get<std::string>()
			),
			.sender = twitch::User(
				std::stoi(json["chatter_user_id"].get<std::string>()),
				json["chatter_user_name"].get<std::string>(),
				json["chatter_user_login"].get<std::string>()
			),
			.messageId = json["message_id"].get<std::string>()
		};
		this->fire(label, cmdArgs);
	}
}