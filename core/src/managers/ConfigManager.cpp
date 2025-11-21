#include "core/pch.h"
#include "core/managers/ConfigManager.h"

void ConfigManager::setConfig(const std::shared_ptr<Config>& config) {
    config_ = config;
}

std::shared_ptr<Config> ConfigManager::getConfig() const {
    return config_;
}

void ConfigManager::clearConfig() {
    config_.reset();
}

bool ConfigManager::saveConfig(const std::string& name, const std::shared_ptr<Config>& config) {
    if (!config) return false;
    configs_[name] = config;
    return true;
}

std::optional<std::shared_ptr<Config>> ConfigManager::loadConfig(const std::string& name) const {
    auto it = configs_.find(name);
    if (it == configs_.end()) return std::nullopt;
    return it->second;
}

std::vector<std::string> ConfigManager::listConfigs() const {
    std::vector<std::string> out;
    out.reserve(configs_.size());
    for (const auto& kv : configs_) out.push_back(kv.first);
    return out;
}

bool ConfigManager::deleteConfig(const std::string& name) {
    auto it = configs_.find(name);
    if (it == configs_.end()) return false;
    configs_.erase(it);
    if (defaultConfigName_ == name) defaultConfigName_.clear();
    return true;
}

void ConfigManager::setDefaultConfig(const std::string& name) {
    defaultConfigName_ = name;
}

std::optional<std::shared_ptr<Config>> ConfigManager::getDefaultConfig() const {
    if (defaultConfigName_.empty()) return std::nullopt;
    auto it = configs_.find(defaultConfigName_);
    if (it == configs_.end()) return std::nullopt;
    return it->second;
}

std::string ConfigManager::getDefaultConfigName() const {
    return defaultConfigName_;
}