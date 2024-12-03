#pragma once
#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

namespace sql {
	class Connector {
	public:
		Connector(std::string dbFile);
		SQLite::Statement createStatement(std::string query, uint32_t broadcasterId);
		SQLite::Statement createNormalStatement(std::string query);
	private:
		SQLite::Database db;
	};
}