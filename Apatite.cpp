#include "Apatite.h"

Apatite::Apatite() {
    this->twitchAPIConnector = new TwitchAPIConnector();
    this->authConfig = AuthConfig();
    this->cmdManager = new CmdManager();
    this->modCmds = new ModCmds();
}

Apatite::~Apatite() {
    delete this->twitchAPIConnector;
    ManagedSingleton<Tokens>::destroyInstance();
    delete this->cmdManager;
    delete this->modCmds;
}

Apatite& Apatite::fetchInstance() {
    if (ManagedSingleton<Apatite>::instance() == nullptr) {
        ManagedSingleton<Apatite>::createInstance();
    }
    return *(ManagedSingleton<Apatite>::instance());
}

void Apatite::restart() {
    this->authConfig.load("auth.yml");
    this->authConfig.save();
    ManagedSingleton<Tokens>::createInstance();
    if (!this->twitchAPIConnector->connect()) {
        spdlog::error("Connection failed");
        return;
    }
    this->cmdManager->hookSubscription();
    this->modCmds->init();
    Tokens::fetchInstance().save();
    this->run();
}

void Apatite::run() {
    this->running = true;
    while (this->running) {
        this->twitchAPIConnector->run();
    }
}

void Apatite::stop() {
    this->running = false;
}

int main(int argc, char* argv[]) {
    #ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    #endif
    Apatite::fetchInstance().restart();
    ManagedSingleton<Apatite>::destroyInstance();
}