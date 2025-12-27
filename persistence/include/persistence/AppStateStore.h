#pragma once

#include <memory>

#include "core/models/AppState.h"
#include "persistence/AppStateStoreResult.h"

class SqliteDb;

class AppStateStore {
public:
    explicit AppStateStore(std::shared_ptr<SqliteDb> db);

    AppState load();
    AppStateStoreResult save(const AppState& state);

private:
    std::shared_ptr<SqliteDb> db_;
};
