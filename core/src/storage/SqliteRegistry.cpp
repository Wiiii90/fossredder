#include "core/storage/SqliteRegistry.h"

#include <sqlite3.h>
#include <stdexcept>

namespace core::storage {

struct SqliteRegistry::Impl { sqlite3* db = nullptr; };

static void exec_noexcept(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
}

SqliteRegistry::SqliteRegistry(const std::string& dbPath)
    : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) {
        sqlite3_close(pimpl_->db);
        pimpl_.reset();
        throw std::runtime_error("failed to open registry db");
    }
    exec_noexcept(pimpl_->db, "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY KEY, value TEXT);");
}

SqliteRegistry::~SqliteRegistry() {
    if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db);
}

std::optional<std::string> SqliteRegistry::getLatest() const {
    if (!pimpl_ || !pimpl_->db) return std::nullopt;
    const char* sql = "SELECT value FROM configs WHERE name = '__latest__' LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    std::optional<std::string> out;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* v = sqlite3_column_text(stmt, 0);
        if (v) out = std::string(reinterpret_cast<const char*>(v));
    }
    sqlite3_finalize(stmt);
    return out;
}

void SqliteRegistry::setLatest(const std::string& path) {
    if (!pimpl_ || !pimpl_->db) return;
    const char* sql = "INSERT OR REPLACE INTO configs (name, value) VALUES ('__latest__', ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

}
