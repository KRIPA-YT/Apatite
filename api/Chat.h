#pragma once
#include <string>

namespace twitch {
	class Chat {
	public:
		Chat();

		bool sendMessage(std::string message);
	};
}