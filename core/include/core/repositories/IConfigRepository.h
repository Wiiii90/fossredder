#pragma once

/**
 * @file core/include/core/repositories/IConfigRepository.h
 * @brief Repository interface for named configuration persistence.
 *
 * Defines the persistence port used by higher-level managers to store and
 * retrieve named configuration blobs. Implementations live in the
 * persistence/ module (e.g. SqliteConfigRepository).
 */

#include <string>
#include <optional>
#include <vector>
#include <memory>

class Config;

class IConfigRepository {
public:
    virtual ~IConfigRepository() = default;

    /**
     * @brief Save or replace a named configuration.
     * @param name Logical name for the configuration.
     * @param config Shared pointer to the configuration object.
     * @return true on success.
     */
    virtual bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) = 0;

    /**
     * @brief Load a named configuration.
     * @param name Logical name of the configuration to load.
     * @return Optional shared_ptr to the Config when present.
     */
    virtual std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const = 0;

    /**
     * @brief List all available configuration names.
     * @return Vector of configuration names.
     */
    virtual std::vector<std::string> listConfigs() const = 0;

    /**
     * @brief Delete a named configuration.
     * @param name Name of the configuration to delete.
     * @return true if an entry was removed.
     */
    virtual bool deleteConfig(const std::string& name) = 0;

    /**
     * @brief Set the default configuration name persisted by the repository.
     * @param name Name of the configuration to mark as default.
     */
    virtual void setDefaultConfig(const std::string& name) = 0;

    /**
     * @brief Retrieve the currently configured default configuration.
     * @return Optional shared_ptr to the default Config.
     */
    virtual std::optional<std::shared_ptr<Config>> getDefaultConfig() const = 0;

    /**
     * @brief Return the name of the default configuration (may be empty).
     * @return Default configuration name string.
     */
    virtual std::string getDefaultConfigName() const = 0;
};
