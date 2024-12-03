#pragma once
#include <string>

namespace twitch {
	class Chat {
	public:
		Chat();

		bool sendMessage(std::string message);
		bool reply(std::string message, std::string messageId);
	};
}