#include "persistence/repositories/SqliteActorRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Actor.h"

struct SqliteActorRepository::Impl { sqlite3* db = nullptr; };

SqliteActorRepository::SqliteActorRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open sqlite database");
    }
    const char* sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS actors ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT UNIQUE,"
        "type TEXT,"
        "description TEXT"
        ");";
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

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

void SqliteActorRepository::addActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    const char* sql = "INSERT INTO actors (name, type, description) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->description.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        actor->id = std::to_string(sqlite3_last_insert_rowid(pimpl_->db));
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    const char* sql = "SELECT id, name, type, description FROM actors ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);

        auto a = std::make_shared<Actor>(
            name ? reinterpret_cast<const char*>(name) : std::string(),
            type ? reinterpret_cast<const char*>(type) : std::string(),
            desc ? reinterpret_cast<const char*>(desc) : std::string()
        );
        a->id = std::to_string(id);
        out.push_back(std::move(a));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const {
    long long aid = toLL(id);
    if (aid <= 0) return std::nullopt;

    const char* sql = "SELECT id, name, type, description FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, aid);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    long long rid = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* type = sqlite3_column_text(stmt, 2);
    const unsigned char* desc = sqlite3_column_text(stmt, 3);

    auto a = std::make_shared<Actor>(
        name ? reinterpret_cast<const char*>(name) : std::string(),
        type ? reinterpret_cast<const char*>(type) : std::string(),
        desc ? reinterpret_cast<const char*>(desc) : std::string()
    );
    a->id = std::to_string(rid);

    sqlite3_finalize(stmt);
    return a;
}

void SqliteActorRepository::removeActor(const std::string& id) {
    long long aid = toLL(id);
    if (aid <= 0) return;

    const char* sql = "DELETE FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, aid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteActorRepository::updateActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    long long aid = toLL(actor->id);
    if (aid <= 0) return;

    const char* sql = "UPDATE actors SET name = ?, type = ?, description = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, aid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
