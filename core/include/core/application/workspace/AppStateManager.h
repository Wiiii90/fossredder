/**
 * @file core/include/core/application/AppStateManager.h
 * @brief Loader/saver for core::domain::WorkspaceState using repository abstractions.
 *
 * Responsible for converting between repository data and the in-memory
 * core::domain::WorkspaceState representation. Provides load/save and validation/rehydration logic.
 */

#pragma once

#include <memory>

namespace core::domain {
struct WorkspaceState;
}

namespace core::storage {
struct RepositoryBundle;
}

namespace core::application {

/**
 * @brief Loads and saves aggregate application state via repository abstractions.
 */
class AppStateManager {
public:
    using Repositories = core::storage::RepositoryBundle;

    /** @brief Creates a manager over the supplied repository bundle. */
    explicit AppStateManager(Repositories repos);
    ~AppStateManager();

    AppStateManager(const AppStateManager&) = delete;
    AppStateManager& operator=(const AppStateManager&) = delete;
    AppStateManager(AppStateManager&&) noexcept;
    AppStateManager& operator=(AppStateManager&&) noexcept;

    /** @brief Loads, rehydrates and validates the application state. */
    core::domain::WorkspaceState load();
    /** @brief Projects, validates and persists the application state. */
    void save(const core::domain::WorkspaceState& state);

    /** @brief Enables or disables strict validation during load and save. */
    void setStrictValidation(bool enabled) noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
