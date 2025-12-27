#pragma once

#include <sqlite3.h>

class SqliteTransaction {
public:
    explicit SqliteTransaction(sqlite3* db);
    ~SqliteTransaction();

    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;

    void commit();

private:
    sqlite3* db_;
    bool committed_ = false;
};
