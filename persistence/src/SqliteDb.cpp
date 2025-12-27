#include "persistence/SqliteDb.h"

#include <sqlite3.h>
#include <stdexcept>

#include "persistence/SqliteSchema.h"

SqliteDb::SqliteDb(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    SqliteSchema::ensure(db_);
}

SqliteDb::~SqliteDb() {
    if (db_) sqlite3_close(db_);
}
