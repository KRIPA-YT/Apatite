#include "Apatite.h"


Apatite::~Apatite() {
    ManagedSingleton<twitch::Tokens>::destroyInstance();
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
    ManagedSingleton<twitch::Tokens>::createInstance();
    if (!this->twitchAPIConnector.connect()) {
        spdlog::error("Connection failed");
        return;
    }
    this->cmdManager.hookSubscription();
    this->modCmds.init();
    twitch::Tokens::fetchInstance().save();
    this->run();
}

void Apatite::run() {
    this->running = true;
    while (this->running) {
        this->twitchAPIConnector.run();
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