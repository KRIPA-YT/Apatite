#pragma once
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>

using json = nlohmann::json;

struct CommandArgs {
	std::vector<std::string> args;
	uint64_t broadcasterUserId;
	uint64_t chatterUserId;
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
	CmdManager();

	void hook(std::string, Command);
	void unhook(std::string);

	void fire(std::string, CommandArgs args);
private:
	CmdHandlerMap cmdHandlerMap;

	void onMessage(json json);
};