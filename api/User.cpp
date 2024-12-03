#include "User.h"
#include <algorithm>
#include <cpr/cpr.h>
#include "../Apatite.h"
#include "Request.h"
#include <spdlog/spdlog.h>

namespace twitch {
	constexpr auto MAX_SECONDS_SINCE_LAST_UPDATE = 60 * 5;

	User::User(const uint32_t id) : id_(id) {
		auto fUQuery = "SELECT name, login FROM users WHERE id=? AND strftime('%s', 'now') - strftime('%s', last_updated) < ?";
		auto fetchUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(fUQuery);
		fetchUser.bind(1, id);
		fetchUser.bind(2, MAX_SECONDS_SINCE_LAST_UPDATE);
		if (fetchUser.executeStep()) {
			this->name_ = fetchUser.getColumn(0).getString();
			this->login_ = fetchUser.getColumn(1).getString();
			return;
		}

		Request request = Request(GET, "users", Tokens::fetchInstance().botUserAccess);
		request.setParameters({ {"id", std::to_string(id)} });
		auto userData = request.request()["data"][0];
		this->name_ = userData["display_name"].get<std::string>();
		this->login_ = userData["login"].get<std::string>();

		auto pUQuery = "INSERT OR REPLACE INTO users (id, name, login, last_updated) VALUES (?, ?, ?, DATETIME('now'))";
		auto putUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(pUQuery);
		putUser.bind(1, this->id);
		putUser.bind(2, this->name);
		putUser.bind(3, this->login);
		putUser.exec();
	}

	User::User(const std::string login) : login_(login) {
		auto fUQuery = "SELECT id, name FROM users WHERE login=? AND strftime('%s', 'now') - strftime('%s', last_updated) < ?";
		auto fetchUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(fUQuery);
		fetchUser.bind(1, login);
		fetchUser.bind(2, MAX_SECONDS_SINCE_LAST_UPDATE);
		if (fetchUser.executeStep()) {
			this->id_ = fetchUser.getColumn(0).getInt();
			this->name_ = fetchUser.getColumn(1).getString();
			return;
		}

		Request request = Request(GET, "users", Tokens::fetchInstance().botUserAccess);
		request.setParameters({{"login", login}});
		auto userData = request.request()["data"][0];
		this->id_ = std::stoi(userData["id"].get<std::string>());
		this->name_ = userData["display_name"].get<std::string>();

		auto pUQuery = "INSERT OR REPLACE INTO users (id, name, login, last_updated) VALUES (?, ?, ?, DATETIME('now'))";
		auto putUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(pUQuery);
		putUser.bind(1, this->id);
		putUser.bind(2, this->name);
		putUser.bind(3, this->login);
		putUser.exec();
	}

	User::User(const User& user) : id_(user.id), name_(user.name), login_(user.login) {
		auto pUQuery = "INSERT OR REPLACE INTO users (id, name, login, last_updated) VALUES (?, ?, ?, DATETIME('now'))";
		auto putUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(pUQuery);
		putUser.bind(1, this->id);
		putUser.bind(2, this->name);
		putUser.bind(3, this->login);
		putUser.exec();
	}

	User::User(const uint32_t id, std::string name, std::string login) : id_(id), name_(name), login_(login) {
		auto pUQuery = "INSERT OR REPLACE INTO users (id, name, login, last_updated) VALUES (?, ?, ?, DATETIME('now'))";
		auto putUser = Apatite::fetchInstance().sqlConnector.createNormalStatement(pUQuery);
		putUser.bind(1, this->id);
		putUser.bind(2, this->name);
		putUser.bind(3, this->login);
		putUser.exec();
	}

	bool User::isMod(const Broadcaster& broadcaster) const {
		return *this == broadcaster || std::find(broadcaster.mods.begin(), broadcaster.mods.end(), *this) != broadcaster.mods.end();
	}

	bool User::isSudoer(const Broadcaster& broadcaster) const {
		return *this == broadcaster || std::find(broadcaster.sudoers.begin(), broadcaster.sudoers.end(), *this) != broadcaster.sudoers.end();
	}

	bool User::operator==(const User& other) const {
		return other.id == this->id
			&& other.login == this->login
			&& other.name == this->name;
	}

	User& User::operator=(const User& other) {
		this->id_ = other.id;
		this->login_ = other.login;
		this->name_ = other.name;
		return *this;
	}

	Broadcaster::Broadcaster(const User& user) : User(user) {
		auto broadcaster = std::find(broadcasters.begin(), broadcasters.end(), user);
		if (broadcaster != broadcasters.end()) {
			this->mods = (*broadcaster).mods;
			this->sudoers = (*broadcaster).sudoers;
			return;
		}
		this->requestModerators();
		this->requestSudoers();
		broadcasters.push_back(*this);
	}

	Broadcaster::Broadcaster(const uint32_t userId) : User(userId) {
		auto broadcaster = std::find_if(broadcasters.begin(), broadcasters.end(), [&](const Broadcaster& b) { return b.id == userId; });
		if (broadcaster != broadcasters.end()) {
			this->mods = (*broadcaster).mods;
			this->sudoers = (*broadcaster).sudoers;
			return;
		}
		this->requestModerators();
		this->requestSudoers();
		broadcasters.push_back(*this);
	}

	Broadcaster::Broadcaster(const std::string login) : User(login) {
		auto broadcaster = std::find_if(broadcasters.begin(), broadcasters.end(), [&](const Broadcaster& b) { return b.login == login; });
		if (broadcaster != broadcasters.end()) {
			this->mods = (*broadcaster).mods;
			this->sudoers = (*broadcaster).sudoers;
			return;
		}
		this->requestModerators();
		this->requestSudoers();
		broadcasters.push_back(*this);
	}

	Broadcaster::Broadcaster(const uint32_t id, const std::string name, const std::string login) : User(id, name, login) {
		auto broadcaster = std::find(broadcasters.begin(), broadcasters.end(), User(id, name, login));
		if (broadcaster != broadcasters.end()) {
			this->mods = (*broadcaster).mods;
			this->sudoers = (*broadcaster).sudoers;
			return;
		}
		this->requestModerators();
		this->requestSudoers();
		broadcasters.push_back(*this);
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
				this->mods.emplace_back(User(std::stoi(it["user_id"].get<std::string>()), it["user_name"], it["user_login"]));
			}
			if (response.contains("pagination") && !response["pagination"].is_null() && response["pagination"].contains("cursor")) {
				cursor = response["pagination"]["cursor"].get<std::string>();
			}
		} while (cursor != "");
	}

	void Broadcaster::requestSudoers() {
		spdlog::debug("Requesting sudoers");
		Apatite::fetchInstance().sqlConnector.createStatement("CREATE TABLE IF NOT EXISTS {} (id INTEGER PRIMARY KEY, sudo INTEGER)", this->id).exec();
		auto sudoQuery = Apatite::fetchInstance().sqlConnector.createStatement("SELECT id FROM {} WHERE sudo=TRUE", this->id);
		while (sudoQuery.executeStep()) {
			this->sudoers.emplace_back(User(sudoQuery.getColumn(0).getUInt()));
		}
	} // TODO: Save sudoers in Destructor

	std::vector<Broadcaster> Broadcaster::broadcasters{};
}