#include "persistence/repositories/SqliteActorRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include <unordered_map>
#include "core/models/Actor.h"
#include "persistence/SqliteDb.h"

struct SqliteActorRepository::Impl { std::shared_ptr<SqliteDb> db; };

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

static void storeAliases(sqlite3* db, long long actorId, const std::vector<std::string>& aliases) {
    if (!db || actorId <= 0) return;

    sqlite3_stmt* del = nullptr;
    if (sqlite3_prepare_v2(db, "DELETE FROM actor_aliases WHERE actor_id = ?;", -1, &del, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(del, 1, actorId);
        sqlite3_step(del);
    }
    sqlite3_finalize(del);

    if (aliases.empty()) return;

    sqlite3_stmt* ins = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO actor_aliases (actor_id, alias) VALUES (?, ?);", -1, &ins, nullptr) != SQLITE_OK) {
        sqlite3_finalize(ins);
        return;
    }

    for (const auto& a : aliases) {
        if (a.empty()) continue;
        sqlite3_reset(ins);
        sqlite3_clear_bindings(ins);
        sqlite3_bind_int64(ins, 1, actorId);
        sqlite3_bind_text(ins, 2, a.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(ins);
    }

    sqlite3_finalize(ins);
}

static void loadAliases(sqlite3* db, std::vector<std::shared_ptr<Actor>>& actors) {
    if (!db || actors.empty()) return;

    std::unordered_map<long long, std::shared_ptr<Actor>> byId;
    byId.reserve(actors.size());
    for (auto& a : actors) {
        if (!a) continue;
        long long id = toLL(a->id);
        if (id > 0) byId.emplace(id, a);
    }

    const char* sql = "SELECT actor_id, alias FROM actor_aliases ORDER BY actor_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long actorId = sqlite3_column_int64(stmt, 0);
        const unsigned char* alias = sqlite3_column_text(stmt, 1);
        auto it = byId.find(actorId);
        if (it == byId.end()) continue;
        if (alias) it->second->aliases.emplace_back(reinterpret_cast<const char*>(alias));
    }

    sqlite3_finalize(stmt);
}

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

    const char* sql = "INSERT INTO actors (name, type, description) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->description.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        long long id = sqlite3_last_insert_rowid(pimpl_->db->handle());
        actor->id = std::to_string(id);
        storeAliases(pimpl_->db->handle(), id, actor->aliases);
    }
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    const char* sql = "SELECT id, name, type, description FROM actors ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);

        auto a = std::make_shared<Actor>();
        a->id = std::to_string(id);
        a->name = name ? reinterpret_cast<const char*>(name) : std::string();
        a->type = type ? reinterpret_cast<const char*>(type) : std::string();
        a->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
        out.push_back(std::move(a));
    }

    sqlite3_finalize(stmt);
    loadAliases(pimpl_->db->handle(), out);
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const {
    long long aid = toLL(id);
    if (aid <= 0) return std::nullopt;

    const char* sql = "SELECT id, name, type, description FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int64(stmt, 1, aid);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }

    long long rid = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* type = sqlite3_column_text(stmt, 2);
    const unsigned char* desc = sqlite3_column_text(stmt, 3);

    auto a = std::make_shared<Actor>();
    a->id = std::to_string(rid);
    a->name = name ? reinterpret_cast<const char*>(name) : std::string();
    a->type = type ? reinterpret_cast<const char*>(type) : std::string();
    a->description = desc ? reinterpret_cast<const char*>(desc) : std::string();

    sqlite3_finalize(stmt);

    std::vector<std::shared_ptr<Actor>> tmp;
    tmp.push_back(a);
    loadAliases(pimpl_->db->handle(), tmp);

    return a;
}

void SqliteActorRepository::removeActor(const std::string& id) {
    long long aid = toLL(id);
    if (aid <= 0) return;

    const char* sql = "DELETE FROM actors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
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
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, actor->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, actor->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, actor->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, aid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    storeAliases(pimpl_->db->handle(), aid, actor->aliases);
}

void SqliteActorRepository::upsertActor(const std::shared_ptr<Actor>& actor) {
    if (!actor) return;
    long long aid = toLL(actor->id);

    if (aid > 0) {
        updateActor(actor);
        return;
    }
    addActor(actor);
}

void SqliteActorRepository::clearActors() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM actors;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteActorRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
