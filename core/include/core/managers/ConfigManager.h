#pragma once

#include "core/managers/IConfigManager.h"
#include <unordered_map>
#include <memory>

class Config;

class ConfigManager : public IConfigManager {
public:
    bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) override;
    std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const override;
    std::vector<std::string> listConfigs() const override;
    bool deleteConfig(const std::string& name) override;
    void setDefaultConfig(const std::string& name) override;
    std::optional<std::shared_ptr<Config>> getDefaultConfig() const override;
    std::string getDefaultConfigName() const override;

    void setConfig(const std::shared_ptr<Config>& config);
    std::shared_ptr<Config> getConfig() const;
    void clearConfig();

private:
    std::unordered_map<std::string, std::shared_ptr<Config>> configs_;
    std::string defaultConfigName_;
    std::shared_ptr<Config> config_;
};