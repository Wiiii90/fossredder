#include "persistence/repositories/SqliteActorRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "persistence/Uuid.h"
#include "core/models/Actor.h"
#include "persistence/SqliteDb.h"

struct SqliteActorRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteActorRepository::SqliteActorRepository(const std::string& dbPath)
    : SqliteActorRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqliteActorRepository::SqliteActorRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteActorRepository::~SqliteActorRepository() = default;

void SqliteActorRepository::addActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    if (actor->id.empty()) actor->id = persistence::generateUuid();

    const char* sql = "INSERT INTO actors (id, name, type, description) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, actor->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, actor->description.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    const char* sql = "SELECT id, name, type, description FROM actors ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);

        auto a = std::make_shared<Actor>();
        a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        a->name = name ? reinterpret_cast<const char*>(name) : std::string();
        a->type = type ? reinterpret_cast<const char*>(type) : std::string();
        a->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
        out.push_back(std::move(a));
    }

    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const {
    if (id.empty()) return std::nullopt;

    const char* sql = "SELECT id, name, type, description FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }

    const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* type = sqlite3_column_text(stmt, 2);
    const unsigned char* desc = sqlite3_column_text(stmt, 3);

    auto a = std::make_shared<Actor>();
    a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    a->name = name ? reinterpret_cast<const char*>(name) : std::string();
    a->type = type ? reinterpret_cast<const char*>(type) : std::string();
    a->description = desc ? reinterpret_cast<const char*>(desc) : std::string();

    sqlite3_finalize(stmt);
    return a;
}

void SqliteActorRepository::removeActor(const std::string& id) {
    if (id.empty()) return;

    const char* sql = "DELETE FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteActorRepository::updateActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    if (actor->id.empty()) return;

    const char* sql = "UPDATE actors SET name = ?, type = ?, description = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, actor->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteActorRepository::upsertActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    if (actor->id.empty()) { addActor(actor); return; }
    updateActor(actor);
    if (pimpl_ && pimpl_->db && sqlite3_changes(pimpl_->db->handle()) == 0) {
        // nothing updated -> try insert
        addActor(actor);
    }
}

void SqliteActorRepository::clearActors() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM actors;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteActorRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
