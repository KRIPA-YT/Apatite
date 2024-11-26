#include "User.h"
#include <algorithm>
#include <cpr/cpr.h>
#include "../Apatite.h"
#include "APIRequest.h"

namespace twitch {
	User::User(const uint64_t id) : id_(id) {
		Request request = Request(GET, "users", Tokens::fetchInstance().botUserAccess);
		request.setParameters({ {"id", std::to_string(id)} });
		auto userData = request.request()["data"][0];
		this->name_ = userData["display_name"].get<std::string>();
		this->login_ = userData["login"].get<std::string>();
	}

	User::User(const std::string login) : login_(login) {
		Request request = Request(GET, "users", Tokens::fetchInstance().botUserAccess);
		request.setParameters({ {"login", login} });
		auto userData = request.request()["data"][0];
		this->id_ = std::stoi(userData["id"].get<std::string>());
		this->name_ = userData["display_name"].get<std::string>();
	}

	User::User(const User& user) : id_(user.id), name_(user.name), login_(user.login) {

	}

	User::User(const uint64_t id, std::string name, std::string login) : id_(id), name_(name), login_(login) {

	}

	bool User::isMod(const Broadcaster& broadcaster) {
		return *this == broadcaster || std::find(broadcaster.mods.begin(), broadcaster.mods.end(), *this) != broadcaster.mods.end();
	}

	User::operator Broadcaster() const {
		return Broadcaster::of(*this);
	}

	bool User::operator==(const User& other) const {
		return other.id == this->id
			&& other.login == this->login
			&& other.name == this->name;
	}

	Broadcaster::Broadcaster(const User& user) : User(user) {
		this->requestModerators();
	}

	Broadcaster::Broadcaster(const uint64_t userId) : User(userId) {
		this->requestModerators();
	}

	Broadcaster::Broadcaster(const std::string login) : User(login) {
		this->requestModerators();
	}

	Broadcaster::Broadcaster(const uint64_t id, const std::string name, const std::string login) : User(id, name, login) {
		this->requestModerators();
	}

	Broadcaster& Broadcaster::of(const User& user) {
		auto broadcaster = std::find(broadcasters.begin(), broadcasters.end(), user);
		if (broadcaster != broadcasters.end()) {
			return *broadcaster;
		}
		return broadcasters.emplace_back(Broadcaster(user));
	}

	Broadcaster& Broadcaster::of(const uint64_t userId) {
		auto broadcaster = std::find_if(broadcasters.begin(), broadcasters.end(), [&](const Broadcaster& b) { return b.id == userId; });
		if (broadcaster != broadcasters.end()) {
			return *broadcaster;
		}
		return broadcasters.emplace_back(Broadcaster(userId));
	}

	Broadcaster& Broadcaster::of(const std::string login) {
		auto broadcaster = std::find_if(broadcasters.begin(), broadcasters.end(), [&](const Broadcaster& b) { return b.login == login; });
		if (broadcaster != broadcasters.end()) {
			return *broadcaster;
		}
		return broadcasters.emplace_back(Broadcaster(login)); 
	}

	Broadcaster& Broadcaster::of(const uint64_t id, const std::string name, const std::string login) {
		auto broadcaster = std::find(broadcasters.begin(), broadcasters.end(), User(id, name, login));
		if (broadcaster != broadcasters.end()) {
			return *broadcaster;
		}
		return broadcasters.emplace_back(Broadcaster(id, name, login));
	}

	Broadcaster::operator User() const {
		return User(*this);
	}

	void Broadcaster::requestModerators() {
		std::string cursor = "";
		do {
			cpr::Parameters payload = {
				{"broadcaster_id", std::to_string(this->id)}
			};
			if (cursor != "") {
				payload.Add({ "after", cursor });
			}
			Request request = Request(GET, "moderation/moderators", Tokens::fetchInstance().getUserAccess(this->id));
			request.setParameters(payload);
			auto response = request.request();

			for (auto it : response["data"]) {
				this->mods.push_back(User(std::stoi(it["user_id"].get<std::string>()), it["user_name"], it["user_login"]));
			}
			if (response.contains("pagination") && !response["pagination"].is_null() && response["pagination"].contains("cursor")) {
				cursor = response["pagination"]["cursor"].get<std::string>();
			}
		} while (cursor != "");
	}

	std::vector<Broadcaster> Broadcaster::broadcasters{};
}