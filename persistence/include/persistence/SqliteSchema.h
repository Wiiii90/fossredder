#pragma once

#include <string>

struct sqlite3;

class SqliteSchema {
public:
    static void ensure(sqlite3* db);
    static void migrate(sqlite3* db);

private:
    static int getUserVersion(sqlite3* db);
    static void setUserVersion(sqlite3* db, int v);
};
