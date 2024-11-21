#include "Apatite.h"

Apatite::Apatite() {
    this->twitchAPIConnector = new TwitchAPIConnector();
    this->authConfig = new AuthConfig();
}

Apatite::~Apatite() {
    delete this->twitchAPIConnector;
    ManagedSingleton<Tokens>::destroyInstance();
    delete this->authConfig;
    delete this->cmdManager;
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
    if (!this->twitchAPIConnector->connect()) {
        spdlog::error("Connection failed");
        return;
    }
    this->cmdManager = new CmdManager();
    this->run();
}

void Apatite::run() {
    while (true) {
        this->twitchAPIConnector->run();
    }
}


int main(int argc, char* argv[]) {
    #ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    #endif
    Apatite::fetchInstance().restart();
    ManagedSingleton<Apatite>::destroyInstance();
}