#include "persistence/repositories/SqliteActorRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Actor.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqliteActorRepository::Impl { std::shared_ptr<SqliteDb> db; };

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
        "INSERT OR IGNORE INTO actors (id, name, type, description) VALUES (?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, actor->id);  stmt.bindText(2, actor->name);
    stmt.bindText(3, actor->type); stmt.bindText(4, actor->description);
    stmt.step();
}

std::vector<std::shared_ptr<Actor>> SqliteActorRepository::getActors() const {
    std::vector<std::shared_ptr<Actor>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, description FROM actors ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto a = std::make_shared<Actor>();
        a->id = stmt.columnText(0); a->name = stmt.columnText(1);
        a->type = stmt.columnText(2); a->description = stmt.columnText(3);
        out.push_back(std::move(a));
    }
    return out;
}

std::optional<std::shared_ptr<Actor>> SqliteActorRepository::getActorById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, description FROM actors WHERE id = ?;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto a = std::make_shared<Actor>();
    a->id = stmt.columnText(0); a->name = stmt.columnText(1);
    a->type = stmt.columnText(2); a->description = stmt.columnText(3);
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
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE actors SET name = ?, type = ?, description = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, actor->name); stmt.bindText(2, actor->type);
    stmt.bindText(3, actor->description); stmt.bindText(4, actor->id);
    stmt.step();
}

void SqliteActorRepository::upsertActor(const std::shared_ptr<Actor>& actor) {
    if (!actor || actor->id.empty()) return;
    updateActor(actor);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addActor(actor);
}

void SqliteActorRepository::clearActors() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM actors;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteActorRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
