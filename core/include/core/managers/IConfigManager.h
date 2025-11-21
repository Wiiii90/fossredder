#pragma once

#include <string>
#include <optional>
#include <vector>
#include <memory>

class Config;

class IConfigManager {
public:
    virtual ~IConfigManager() = default;

    virtual bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) = 0;
    virtual std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const = 0;
    virtual std::vector<std::string> listConfigs() const = 0;
    virtual bool deleteConfig(const std::string& name) = 0;
    virtual void setDefaultConfig(const std::string& name) = 0;
    virtual std::optional<std::shared_ptr<Config>> getDefaultConfig() const = 0;
    virtual std::string getDefaultConfigName() const = 0;
};