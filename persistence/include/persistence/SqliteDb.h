#pragma once

#include <memory>
#include <string>

#include "core/repositories/ISqlConnectionProvider.h"

struct sqlite3;

class SqliteDb : public ISqlConnectionProvider {
public:
    explicit SqliteDb(const std::string& path);
    ~SqliteDb();

    SqliteDb(const SqliteDb&) = delete;
    SqliteDb& operator=(const SqliteDb&) = delete;

    sqlite3* handle() const noexcept { return db_; }
    sqlite3* sqliteHandle() const noexcept override { return db_; }

private:
    sqlite3* db_ = nullptr;
};
