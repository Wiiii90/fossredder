#include "persistence/repositories/SqliteActorRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Actor.h"

struct SqliteActorRepository::Impl { sqlite3* db = nullptr; };

SqliteActorRepository::SqliteActorRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open sqlite database");
    }
    const char* sql = "CREATE TABLE IF NOT EXISTS actors (name TEXT PRIMARY KEY, type TEXT, description TEXT);";
    char* err = nullptr;
    if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error(std::string("Failed to create actors table: ") + e);
    }
}

SqliteActorRepository::~SqliteActorRepository() {
    if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db);
}

void SqliteActorRepository::addActor(const std::string& name, const std::string& type, const std::string& description) {
    const char* sql = "INSERT OR REPLACE INTO actors (name, type, description) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    const char* sql = "SELECT name, type, description FROM actors ORDER BY name;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* type = sqlite3_column_text(stmt, 1);
        const unsigned char* desc = sqlite3_column_text(stmt, 2);
        auto a = std::make_shared<Actor>(std::string(reinterpret_cast<const char*>(name)), std::string(reinterpret_cast<const char*>(type ? type : (const unsigned char*)"")), std::string(reinterpret_cast<const char*>(desc ? desc : (const unsigned char*)"")));
        out.push_back(a);
    }
    sqlite3_finalize(stmt);
    return out;
}

void SqliteActorRepository::removeActor(const std::string& name) {
    const char* sql = "DELETE FROM actors WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteActorRepository::updateActor(const std::string& name, const std::string& newName, const std::string& newType, const std::string& newDescription) {
    const char* sql = "UPDATE actors SET name = ?, type = ?, description = ? WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, newType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, newDescription.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
