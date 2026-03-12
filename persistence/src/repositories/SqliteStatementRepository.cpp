#include "persistence/repositories/SqliteStatementRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Statement.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>

struct SqliteStatementRepository::Impl {
    std::shared_ptr<SqliteDb> db;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter;
};

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath)
    : SqliteStatementRepository(std::make_shared<SqliteDb>(dbPath), nullptr) {}

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath,
                                                     std::shared_ptr<core::errors::IErrorReporter> er)
    : SqliteStatementRepository(std::make_shared<SqliteDb>(dbPath), std::move(er)) {}

SqliteStatementRepository::SqliteStatementRepository(std::shared_ptr<SqliteDb> db)
    : SqliteStatementRepository(std::move(db), nullptr) {}

SqliteStatementRepository::SqliteStatementRepository(std::shared_ptr<SqliteDb> db,
                                                     std::shared_ptr<core::errors::IErrorReporter> er)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
    pimpl_->errorReporter = std::move(er);
}

SqliteStatementRepository::~SqliteStatementRepository() = default;

void SqliteStatementRepository::addStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO statements (id, name) VALUES (?, ?);");
    if (!stmt) return;
    stmt.bindText(1, statement->id); stmt.bindText(2, statement->name);
    stmt.step();
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name FROM statements ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) {
        auto s = std::make_shared<Statement>();
        s->id = stmt.columnText(0); s->name = stmt.columnText(1);
        out.push_back(std::move(s));
    }
    return out;
}

std::optional<std::shared_ptr<Statement>> SqliteStatementRepository::getStatementById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name FROM statements WHERE id = ?;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    auto s = std::make_shared<Statement>();
    s->id = stmt.columnText(0); s->name = stmt.columnText(1);
    return s;
}

void SqliteStatementRepository::removeStatement(const std::string& id) {
    if (id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM statements WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteStatementRepository::updateStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE statements SET name = ? WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, statement->name); stmt.bindText(2, statement->id);
    stmt.step();
}

void SqliteStatementRepository::upsertStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement || statement->id.empty()) return;
    updateStatement(statement);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addStatement(statement);
}

void SqliteStatementRepository::clearStatements() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM statements;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
