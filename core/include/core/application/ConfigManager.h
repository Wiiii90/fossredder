#pragma once

/**
 * @file core/include/core/application/ConfigManager.h
 * @brief Concrete implementation of IConfigManager used at runtime.
 */

#include "IConfigManager.h"

#include <memory>
#include <unordered_map>

namespace core::application {

class ConfigManager : public IConfigManager {
public:
    bool saveConfig(const std::string& name, const std::shared_ptr<core::domain::Config>& config) override;
    std::optional<std::shared_ptr<core::domain::Config>> loadConfig(const std::string& name) const override;
    std::vector<std::string> listConfigs() const override;
    bool deleteConfig(const std::string& name) override;
    void setDefaultConfig(const std::string& name) override;
    std::optional<std::shared_ptr<core::domain::Config>> getDefaultConfig() const override;
    std::string getDefaultConfigName() const override;

    void setConfig(const std::shared_ptr<core::domain::Config>& config);
    std::shared_ptr<core::domain::Config> getConfig() const;
    void clearConfig();

private:
    std::unordered_map<std::string, std::shared_ptr<core::domain::Config>> configs_;
    std::string defaultConfigName_;
    std::shared_ptr<core::domain::Config> config_;
};

}
