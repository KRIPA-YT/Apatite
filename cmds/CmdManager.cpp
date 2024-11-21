#include "CmdManager.h"
#include "../Apatite.h"
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>

constexpr char PREFIX = '!'; // TODO: Move to config

CmdManager::CmdManager() {
	Apatite::fetchInstance().twitchAPIConnector->hook("channel.chat.message", std::bind(&CmdManager::onMessage, this, std::placeholders::_1));
}

void CmdManager::hook(std::string label, Command command) {
	this->cmdHandlerMap.insert({label, command});
}

void CmdManager::unhook(std::string label) {
	this->cmdHandlerMap.erase(label);
}

void CmdManager::fire(std::string label, CommandArgs args) {
	spdlog::debug("Fired command !{}, with args:", label);
	for (std::string arg : args.args) {
		spdlog::debug(arg);
	}
	if (this->cmdHandlerMap.find(label) == this->cmdHandlerMap.end()) {
		// TODO: Command not found
		return;
	}
	if (this->cmdHandlerMap[label].handler(args)) {
		return;
	}
	// TODO: Print usage message
}

void CmdManager::onMessage(json json) {
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
		.broadcasterUserId = static_cast<uint64_t>(std::stoi(json["broadcaster_user_id"].get<std::string>())),
		.chatterUserId = static_cast<uint64_t>(std::stoi(json["chatter_user_id"].get<std::string>())),
		.message_id = json["message_id"].get<std::string>()
	};
	this->fire(label, cmdArgs);
}
