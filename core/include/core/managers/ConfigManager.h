#pragma once

/**
 * @file core/include/core/managers/ConfigManager.h
 * @brief Concrete implementation of IConfigManager used at runtime.
 *
 * Stores named Config objects in-memory and provides default selection.
 */

#include "core/managers/IConfigManager.h"
#include <unordered_map>
#include <memory>

class Config;

class ConfigManager : public IConfigManager {
public:
    /** Save a named configuration.
     * @return true on success.
     */
    bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) override;

    /** Load a named configuration.
     * @return optional shared_ptr to Config if present.
     */
    std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const override;

    /** List available configuration names. */
    std::vector<std::string> listConfigs() const override;

    /** Delete a named configuration.
     * @return true if deleted or false if not found.
     */
    bool deleteConfig(const std::string& name) override;

    /** Set the named configuration as default. */
    void setDefaultConfig(const std::string& name) override;

    /** Return the default configuration if set. */
    std::optional<std::shared_ptr<Config>> getDefaultConfig() const override;

    /** Return the stored default config name (may be empty). */
    std::string getDefaultConfigName() const override;

    /** In-memory runtime helpers. */
    void setConfig(const std::shared_ptr<Config>& config);
    std::shared_ptr<Config> getConfig() const;
    void clearConfig();

private:
    std::unordered_map<std::string, std::shared_ptr<Config>> configs_;
    std::string defaultConfigName_;
    std::shared_ptr<Config> config_;
};