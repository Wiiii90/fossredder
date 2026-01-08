#include "persistence/SqliteDb.h"

#include <sqlite3.h>
#include <stdexcept>
#include <string>

#include "persistence/SqliteSchema.h"

SqliteDb::SqliteDb(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = "";
        if (db_) {
            const char* msg = sqlite3_errmsg(db_);
            if (msg) err = msg;
            sqlite3_close(db_);
            db_ = nullptr;
        }
        std::string e = std::string("Failed to open sqlite db '") + path + "'" + (err.empty() ? std::string() : std::string(": ") + err);
        throw std::runtime_error(e);
    }
    SqliteSchema::ensure(db_);
}

SqliteDb::~SqliteDb() {
    if (db_) sqlite3_close(db_);
}
