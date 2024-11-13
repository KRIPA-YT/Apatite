#pragma once
#include <string>

struct Tokens {
	std::string access;
	std::string clientId;
	std::string clientSecret;
	std::string refresh;
	int64_t accessExpiry;

	Tokens();
	~Tokens();

	static Tokens& fetchInstance();
};