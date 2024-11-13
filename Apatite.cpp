#include "Apatite.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

Apatite::Apatite() {
    this->authConfig = new AuthConfig();
    this->authConfig->load("auth.yml");
    this->authConfig->save();
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
    ManagedSingleton<Tokens>::createInstance();
    this->twitchAPIConnector = new TwitchAPIConnector();
}


int main(int argc, char* argv[]) {
    Apatite::fetchInstance().restart();
    ManagedSingleton<Apatite>::destroyInstance();
}