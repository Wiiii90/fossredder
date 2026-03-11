#include "core/pch.h"
#include "../../include/core/application/ConfigManager.h"

#include <algorithm>
#include <cctype>

namespace {

bool isValidConfigName(const std::string& name)
{
    return std::any_of(name.begin(), name.end(), [](unsigned char ch) { return std::isspace(ch) == 0; });
}

}

namespace core::application {

void ConfigManager::setConfig(const std::shared_ptr<core::domain::Config>& config)
{
    config_ = config;
}

std::shared_ptr<core::domain::Config> ConfigManager::getConfig() const
{
    return config_;
}

void ConfigManager::clearConfig()
{
    config_.reset();
}

bool ConfigManager::saveConfig(const std::string& name, const std::shared_ptr<core::domain::Config>& config)
{
    if (!config || !isValidConfigName(name)) return false;

    configs_[name] = config;
    return true;
}

std::optional<std::shared_ptr<core::domain::Config>> ConfigManager::loadConfig(const std::string& name) const
{
    if (!isValidConfigName(name)) return std::nullopt;

    const auto it = configs_.find(name);
    if (it == configs_.end()) return std::nullopt;
    return it->second;
}

std::vector<std::string> ConfigManager::listConfigs() const
{
    std::vector<std::string> configs;
    configs.reserve(configs_.size());
    for (const auto& [name, config] : configs_) {
        (void)config;
        configs.push_back(name);
    }
    std::sort(configs.begin(), configs.end());
    return configs;
}

bool ConfigManager::deleteConfig(const std::string& name)
{
    if (!isValidConfigName(name)) return false;

    const auto it = configs_.find(name);
    if (it == configs_.end()) return false;

    configs_.erase(it);
    if (defaultConfigName_ == name) defaultConfigName_.clear();
    return true;
}

void ConfigManager::setDefaultConfig(const std::string& name)
{
    if (!isValidConfigName(name)) {
        defaultConfigName_.clear();
        return;
    }

    if (configs_.find(name) == configs_.end()) {
        defaultConfigName_.clear();
        return;
    }

    defaultConfigName_ = name;
}

std::optional<std::shared_ptr<core::domain::Config>> ConfigManager::getDefaultConfig() const
{
    if (defaultConfigName_.empty()) return std::nullopt;

    const auto it = configs_.find(defaultConfigName_);
    if (it == configs_.end()) return std::nullopt;
    return it->second;
}

std::string ConfigManager::getDefaultConfigName() const
{
    return defaultConfigName_;
}

}
