/**
 * @file persistence/include/persistence/AppStateStore.h
 * @brief Declares SQLite-backed loading and saving of the full application state.
 */

#pragma once

#include <memory>

#include "core/models/AppState.h"
#include "core/models/DeletionImpact.h"

class SqliteDb;

/**
 * @brief Loads and saves the aggregate application state through SQLite repositories.
 */
class AppStateStore {
public:
    /** @brief Creates a state store over an existing SQLite database connection wrapper. */
    explicit AppStateStore(std::shared_ptr<SqliteDb> db);

    /** @brief Loads the complete persisted application state from the database. */
    core::domain::AppState load();
    /** @brief Saves the complete application state and reports deletion impact details. */
    core::domain::DeletionImpact save(const core::domain::AppState& state);

private:
    std::shared_ptr<SqliteDb> db_;
};
