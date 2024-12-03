#include "SQLConnector.h"
#include <format>


namespace sql {
	Connector::Connector(std::string dbFile) : db(SQLite::Database(dbFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)) {
		this->db.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name VARCHAR(25), login VARCHAR(25), last_updated TIMESTAMP);");
	}
	SQLite::Statement Connector::createStatement(std::string query, uint32_t broadcasterId) {
		std::string broadcasterIdString = std::format("\"{}\"", broadcasterId);
		return SQLite::Statement(this->db, std::vformat(query, std::make_format_args(broadcasterIdString)));
	}
	SQLite::Statement Connector::createNormalStatement(std::string query) {
		return SQLite::Statement(this->db, query);
	}
}