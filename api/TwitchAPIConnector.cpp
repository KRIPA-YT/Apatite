#include "TwitchAPIConnector.h"

void TwitchAPIConnector::authenticate() {
	TwitchAPIAuthenticationServer authServer = TwitchAPIAuthenticationServer();
	authServer.authenticate();
}

void TwitchAPIConnector::connect() {
	this->authenticate();
	this->connect();
}