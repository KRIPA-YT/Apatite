#include "Apatite.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

Apatite::Apatite() {
    this->twitchAPIConnector = new TwitchAPIConnector();
    this->authConfig = new AuthConfig();
}

Apatite::~Apatite() {
    delete this->twitchAPIConnector;
    delete this->authConfig;
}

Apatite& Apatite::fetchInstance() {
    if (ManagedSingleton<Apatite>::instance() == nullptr) {
        ManagedSingleton<Apatite>::createInstance();
    }
    return *(ManagedSingleton<Apatite>::instance());
}

void Apatite::restart() {
    this->authConfig->load("auth.yml");
    this->authConfig->save();
    ManagedSingleton<Tokens>::createInstance();
    this->twitchAPIConnector->connect();
}


int main(int argc, char* argv[]) {
    Apatite::fetchInstance().restart();
    ManagedSingleton<Apatite>::destroyInstance();
}