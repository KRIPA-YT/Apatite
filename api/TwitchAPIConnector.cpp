#include "TwitchAPIConnector.h"

TwitchAPIConnector::TwitchAPIConnector() {
	this->authenticate();
	this->connect();
}

TwitchAPIConnector::~TwitchAPIConnector() {

}

void TwitchAPIConnector::authenticate() {
	TwitchAPIAuthenticationServer* authServer = new TwitchAPIAuthenticationServer();
	delete authServer;
}

void TwitchAPIConnector::connect() {
}