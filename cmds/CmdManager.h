#pragma once
#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <string>
#include "../api/User.h"

namespace cmds {
	using json = nlohmann::json;

	struct CommandArgs {
		std::vector<std::string> args;
		const twitch::Broadcaster& broadcaster;
		const twitch::User& sender;
		std::string messageId;
	};
	struct Command {
		std::string description = "";
		std::string usage = "";
		std::function<bool(const CommandArgs&)> handler;
	};

	using CmdHandlerMap = std::map<std::string, Command>;

	class CmdManager {
	public:
		void hookSubscription();

		void hook(std::string, Command);
		void unhook(std::string);

		void fire(std::string, const CommandArgs& args);
		void elevFire(std::string, const CommandArgs& args);
		bool isModCmdAllowed(const twitch::User& user, const twitch::Broadcaster& broadcaster);
	private:
		CmdHandlerMap cmdHandlerMap;
		std::vector<uint64_t> currentSudoers;

		void onMessage(const json& json);
	};
}