#pragma once
#include <string>
#include <condition_variable>
#include <mutex>

class TwitchChat {
public:
    TwitchChat();

	bool sendMessage(std::string message);
};