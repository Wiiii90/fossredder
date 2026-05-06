#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Actor.h"
#include "persistence/SqliteDb.h"
#include "core/utils/Time.h"
#include <sqlite3.h>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <stdexcept>

struct SqliteActorRepository::Impl { std::shared_ptr<SqliteDb> db; };

namespace {

struct AliasStats {
    int hitCount = 1;
    std::string lastUsedAt;
    std::string createdAt;
    std::string updatedAt;
};

std::string aliasValue(const core::domain::AliasUsage& usage)
{
    return usage.alias.value;
}

core::domain::AliasUsage makeAliasUsage(const std::string& alias,
                                       int hitCount,
                                       const std::string& createdAt,
                                       const std::string& updatedAt,
                                       const std::string& lastUsedAt)
{
    core::domain::AliasUsage usage;
    usage.alias.value = alias;
    usage.alias.source = alias;
    usage.alias.createdAt = createdAt;
    usage.alias.updatedAt = updatedAt;
    usage.hitCount = hitCount;
    usage.createdAt = createdAt;
    usage.updatedAt = updatedAt;
    usage.lastUsedAt = lastUsedAt;
    return usage;
}

std::vector<core::domain::AliasUsage> collectAliasUsage(const core::domain::Actor& actor)
{
    if (!actor.aliasUsage.empty()) return actor.aliasUsage;

    std::vector<core::domain::AliasUsage> out;
    out.reserve(actor.aliases.size());
    for (const auto& alias : actor.aliases) {
        if (alias.value.empty()) continue;
        out.push_back(makeAliasUsage(alias.value, 1, alias.createdAt, alias.updatedAt, {}));
    }
    return out;
}

std::unordered_map<std::string, AliasStats> loadActorAliasStats(sqlite3* db, const std::string& actorId)
{
    std::unordered_map<std::string, AliasStats> out;
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM actor_aliases WHERE actor_id = ?;");
    if (!stmt) return out;

    stmt.bindText(1, actorId);
    while (stmt.step() == SQLITE_ROW) {
        AliasStats stats;
        stats.hitCount = stmt.columnInt(1);
        stats.createdAt = stmt.columnText(2);
        stats.updatedAt = stmt.columnText(3);
        stats.lastUsedAt = stmt.columnText(4);
        out.emplace(stmt.columnText(0), std::move(stats));
    }

    return out;
}

void loadActorAliases(sqlite3* db, core::domain::Actor& actor)
{
    persistence::StmtGuard stmt(db, "SELECT alias, hit_count, created_at, updated_at, last_used_at FROM actor_aliases WHERE actor_id = ? ORDER BY hit_count DESC, last_used_at DESC, alias ASC;");
    if (!stmt) return;

    stmt.bindText(1, actor.id);
    while (stmt.step() == SQLITE_ROW) {
        core::domain::AliasUsage usage;
        usage.alias.value = stmt.columnText(0);
        usage.alias.source = usage.alias.value;
        usage.alias.createdAt = stmt.columnText(2);
        usage.alias.updatedAt = stmt.columnText(3);
        usage.hitCount = stmt.columnInt(1);
        usage.createdAt = stmt.columnText(2);
        usage.updatedAt = stmt.columnText(3);
        usage.lastUsedAt = stmt.columnText(4);
        actor.aliases.push_back(usage.alias);
        actor.aliasUsage.push_back(std::move(usage));
    }
}

void deleteActorAliases(sqlite3* db, const std::string& actorId)
{
    persistence::StmtGuard stmt(db, "DELETE FROM actor_aliases WHERE actor_id = ?;");
    if (!stmt) return;

    stmt.bindText(1, actorId);
    stmt.step();
}

void replaceActorAliases(sqlite3* db,
                         const core::domain::Actor& actor,
                         const std::unordered_map<std::string, AliasStats>& existing)
{
    if (actor.id.empty()) return;

    deleteActorAliases(db, actor.id);

    persistence::StmtGuard stmt(db,
        "INSERT INTO actor_aliases (actor_id, alias, hit_count, created_at, updated_at, last_used_at) VALUES (?, ?, ?, COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP), COALESCE(NULLIF(?, ''), CURRENT_TIMESTAMP));");
    if (!stmt) return;

    std::unordered_set<std::string> seen;
    for (const auto& usage : collectAliasUsage(actor)) {
        if (usage.alias.value.empty() || !seen.insert(usage.alias.value).second) continue;

        const auto it = existing.find(usage.alias.value);
        const int hitCount = it == existing.end() ? std::max(1, usage.hitCount) : std::max(1, it->second.hitCount);
        const std::string createdAt = it == existing.end() || it->second.createdAt.empty() ? usage.createdAt : it->second.createdAt;
        const std::string updatedAt = it == existing.end() || it->second.updatedAt.empty() ? usage.updatedAt : it->second.updatedAt;
        const std::string lastUsedAt = it == existing.end() || it->second.lastUsedAt.empty() ? usage.lastUsedAt : it->second.lastUsedAt;
        stmt.reset();
        stmt.bindText(1, actor.id);
        stmt.bindText(2, usage.alias.value);
        stmt.bindInt(3, hitCount);
        stmt.bindText(4, createdAt);
        stmt.bindText(5, updatedAt);
        stmt.bindText(6, lastUsedAt);
        stmt.step();
    }
}

} // namespace

SqliteActorRepository::SqliteActorRepository(const std::string& dbPath)
    : SqliteActorRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteActorRepository::SqliteActorRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteActorRepository::~SqliteActorRepository() = default;

void SqliteActorRepository::addActor(const std::shared_ptr<Actor>& actor) {
    if (!actor || actor->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO actors (id, name, created_at, updated_at) VALUES (?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, actor->id);  stmt.bindText(2, actor->name);
    stmt.bindText(3, actor->createdAt); stmt.bindText(4, actor->updatedAt);
    if (stmt.step() == SQLITE_DONE) replaceActorAliases(pimpl_->db->handle(), *actor, {});
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM actors ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto a = std::make_shared<Actor>();
        a->id = stmt.columnText(0); a->name = stmt.columnText(1);
        a->createdAt = stmt.columnText(2);
        a->updatedAt = stmt.columnText(3);
        a->aliasUsage.clear();
        loadActorAliases(pimpl_->db->handle(), *a);
        out.push_back(std::move(a));
    }
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, created_at, updated_at FROM actors WHERE id = ?;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto a = std::make_shared<Actor>();
    a->id = stmt.columnText(0); a->name = stmt.columnText(1);
    a->createdAt = stmt.columnText(2);
    a->updatedAt = stmt.columnText(3);
    a->aliasUsage.clear();
    loadActorAliases(pimpl_->db->handle(), *a);
    return a;
}

void SqliteActorRepository::removeActor(const std::string& id) {
    if (id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM actors WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteActorRepository::updateActor(const std::shared_ptr<Actor>& actor) {
    if (!actor || actor->id.empty()) return;
    const auto existing = loadActorAliasStats(pimpl_->db->handle(), actor->id);
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE actors SET name = ?, created_at = COALESCE(NULLIF(created_at, ''), ?), updated_at = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, actor->name); stmt.bindText(2, actor->createdAt); stmt.bindText(3, actor->updatedAt); stmt.bindText(4, actor->id);
    if (stmt.step() == SQLITE_DONE) replaceActorAliases(pimpl_->db->handle(), *actor, existing);
}

void SqliteActorRepository::upsertActor(const std::shared_ptr<Actor>& actor) {
    if (!actor || actor->id.empty()) return;
    updateActor(actor);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addActor(actor);
}

void SqliteActorRepository::clearActors() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM actor_aliases; DELETE FROM actors;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteActorRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
