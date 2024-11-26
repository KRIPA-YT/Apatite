#pragma once
#include <string>
#include <map>
#include <vector>

struct TokenPair {
	std::string access;
	std::string refresh;
	std::vector<std::string> scopes;
};

struct Tokens {
	TokenPair botUserAccess;
	std::string appAccess;
	std::string clientId;
	std::string clientSecret;

	Tokens();
	~Tokens();

	void save();

	TokenPair &getUserAccess(uint64_t userId);

	static Tokens& fetchInstance();
private:
	std::map<uint64_t, TokenPair> userTokenPairs;
};