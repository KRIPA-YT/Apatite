#pragma once
#include <stdint.h>
#include <string>
#include <vector>

namespace twitch {
	struct Broadcaster;
	struct User;

	struct User {
	private:
		uint32_t id_;
		std::string name_;
		std::string login_;
	public:
		const uint32_t& id = id_;
		const std::string& name = name_;
		const std::string& login = login_;

		User(const uint32_t id);
		User(const std::string login);
		User(const User& user);
		User(const uint32_t id, std::string name, std::string login);

		bool isMod(const Broadcaster& broadcaster) const;
		bool isSudoer(const Broadcaster& broadcaster) const;

		bool operator==(const User& other) const;
		User& operator=(const User& other);
	};

	struct Broadcaster : public User {
		Broadcaster(const User& user);
		Broadcaster(const uint32_t id);
		Broadcaster(const std::string login);
		Broadcaster(const uint32_t id, const std::string name, const std::string login);

		std::vector<User> mods;
		std::vector<User> sudoers;
	private:
		void requestModerators();
		void requestSudoers();

		static std::vector<Broadcaster> broadcasters;
	};
}