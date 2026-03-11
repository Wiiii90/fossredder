#pragma once

/**
 * @file core/include/core/application/IConfigManager.h
 * @brief Abstract interface for configuration persistence and management.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
struct Config;
}

namespace core::application {

class IConfigManager {
public:
    virtual ~IConfigManager() = default;
    virtual bool saveConfig(const std::string& name, const std::shared_ptr<core::domain::Config>& config) = 0;
    virtual std::optional<std::shared_ptr<core::domain::Config>> loadConfig(const std::string& name) const = 0;
    virtual std::vector<std::string> listConfigs() const = 0;
    virtual bool deleteConfig(const std::string& name) = 0;
    virtual void setDefaultConfig(const std::string& name) = 0;
    virtual std::optional<std::shared_ptr<core::domain::Config>> getDefaultConfig() const = 0;
    virtual std::string getDefaultConfigName() const = 0;
};

}
