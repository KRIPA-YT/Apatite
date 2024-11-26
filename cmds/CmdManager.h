#pragma once
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>
#include "../api/User.h"

using json = nlohmann::json;

struct CommandArgs {
	std::vector<std::string> args;
	twitch::Broadcaster& broadcaster;
	twitch::User sender;
	std::string message_id;
};
struct Command {
	std::string description = "";
	std::string usage = "";
	std::function<bool(CommandArgs)> handler;
};

using CmdHandlerMap = std::map<std::string, Command>;

class CmdManager {
public:
	void hookSubscription();

	void hook(std::string, Command);
	void unhook(std::string);

	void fire(std::string, CommandArgs args);
private:
	CmdHandlerMap cmdHandlerMap;

	void onMessage(json json);
};