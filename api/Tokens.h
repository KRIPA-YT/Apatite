#pragma once
#include <string>

struct Tokens {
	std::string userAccess;
	std::string appAccess;
	std::string clientId;
	std::string clientSecret;
	std::string refresh;
	int64_t userAccessExpiry;
	int64_t appAccessExpiry;

	Tokens();
	~Tokens();

	static Tokens& fetchInstance();
};