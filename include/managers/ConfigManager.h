#pragma once
#include "managers/IConfigManager.h"
#include <unordered_map>

class ConfigManager : public IConfigManager {
public:
    bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) override;
    std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const override;
    std::vector<std::string> listConfigs() const override;
    bool deleteConfig(const std::string& name) override;
    void setDefaultConfig(const std::string& name) override;
    std::optional<std::shared_ptr<Config>> getDefaultConfig() const override;
    std::string getDefaultConfigName() const override;

private:
    std::unordered_map<std::string, std::shared_ptr<Config>> configs_;
    std::string defaultConfigName_;
};