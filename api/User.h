#pragma once
#include <stdint.h>
#include <string>
#include <vector>

namespace twitch {
	struct Broadcaster;
	struct User;

	struct User {
	private:
		uint64_t id_;
		std::string name_;
		std::string login_;
	public:
		User(const uint64_t id);
		User(const std::string login);
		User(const User& user);
		User(const uint64_t id, std::string name, std::string login);

		const uint64_t& id = id_;
		const std::string& name = name_;
		const std::string& login = login_;
		bool isMod(const Broadcaster& broadcaster);
		operator Broadcaster() const;
		bool operator==(const User& other) const;
	};

	struct Broadcaster : public User {
		static Broadcaster &of(const User& user);
		static Broadcaster &of(const uint64_t id);
		static Broadcaster &of(const std::string login);
		static Broadcaster &of(const uint64_t id, const std::string name, const std::string login);

		operator User() const;

		std::vector<User> mods;
	private:
		Broadcaster(const User& user);
		Broadcaster(const uint64_t id);
		Broadcaster(const std::string login);
		Broadcaster(const uint64_t id, const std::string name, const std::string login);

		void requestModerators();

		static std::vector<Broadcaster> broadcasters;
	};
}