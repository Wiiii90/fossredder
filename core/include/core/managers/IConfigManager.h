#pragma once

/**
 * @file core/include/core/managers/IConfigManager.h
 * @brief Abstract interface for configuration persistence and management.
 *
 * Provides an abstraction to save/load named configurations and manage the
 * default configuration selection.
 */

#include <string>
#include <optional>
#include <vector>
#include <memory>

class Config;

class IConfigManager {
public:
    virtual ~IConfigManager() = default;

    /**
     * @brief Save or overwrite a named configuration.
     * @param name Logical name for the configuration.
     * @param config Shared pointer to the Config object to persist.
     * @return true on success, false on failure.
     */
    virtual bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) = 0;

    /**
     * @brief Load a named configuration if present.
     * @param name Logical name of the configuration to load.
     * @return Optional shared_ptr to the Config when found.
     */
    virtual std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const = 0;

    /**
     * @brief List all available named configurations.
     * @return Vector of configuration names.
     */
    virtual std::vector<std::string> listConfigs() const = 0;

    /**
     * @brief Delete a named configuration.
     * @param name Name of the configuration to delete.
     * @return true if deletion succeeded (or existed and removed), false otherwise.
     */
    virtual bool deleteConfig(const std::string& name) = 0;

    /**
     * @brief Set the default configuration by name.
     * @param name Name of the configuration to mark as default.
     */
    virtual void setDefaultConfig(const std::string& name) = 0;

    /**
     * @brief Retrieve the currently configured default configuration if any.
     * @return Optional shared_ptr to the default Config.
     */
    virtual std::optional<std::shared_ptr<Config>> getDefaultConfig() const = 0;

    /**
     * @brief Return the name of the default configuration (may be empty).
     * @return Default configuration name string.
     */
    virtual std::string getDefaultConfigName() const = 0;
};