#include "managers/ConfigManager.h"

void ConfigManager::setConfig(const std::shared_ptr<Config>& config) {
    config_ = config;
}

std::shared_ptr<Config> ConfigManager::getConfig() const {
    return config_;
}

void ConfigManager::clearConfig() {
    config_.reset();
}