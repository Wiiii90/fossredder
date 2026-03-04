#include "persistence/repositories/SqliteStatementRepository.h"
#include "core/errors/ErrorReporterRegistry.h"
#include <sqlite3.h>
#include <stdexcept>
#include "persistence/Uuid.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "persistence/SqliteDb.h"

struct SqliteStatementRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath)
    : SqliteStatementRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqliteStatementRepository::SqliteStatementRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteStatementRepository::~SqliteStatementRepository() = default;

static std::string lastInsertIdText(sqlite3* db) {
    // Not reliable for TEXT primary keys; callers should set ids before insert.
    return std::string();
}

void SqliteStatementRepository::addStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;

    if (statement->id.empty()) statement->id = persistence::generateUuid();

    const char* insStmtSql = "INSERT INTO statements (id, name) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insStmtSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_text(stmt, 1, statement->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    core::errors::report({
        core::errors::ErrorSeverity::Info,
        "persistence::SqliteStatementRepository::addStatement",
        std::string("inserted statement id='") + statement->id + "' name='" + statement->name + "'",
        {}
    });
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    const char* selStmtSql = "SELECT id, name FROM statements ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);

        auto s = std::make_shared<Statement>();
        s->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        s->name = name ? reinterpret_cast<const char*>(name) : std::string();

        out.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Statement>> SqliteStatementRepository::getStatementById(const std::string& id) const {
    if (id.empty()) return std::nullopt;

    const char* selStmtSql = "SELECT id, name FROM statements WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    sqlite3_finalize(stmt);

    auto s = std::make_shared<Statement>();
    s->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    s->name = name ? reinterpret_cast<const char*>(name) : std::string();

    return s;
}

void SqliteStatementRepository::removeStatement(const std::string& id) {
    if (id.empty()) return;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM statements WHERE id = ?;";
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteStatementRepository::updateStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;
    if (statement->id.empty()) {
        addStatement(statement);
        return;
    }

    const char* updStmtSql = "UPDATE statements SET name = ? WHERE id = ?;";
    sqlite3_stmt* updStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), updStmtSql, -1, &updStmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(updStmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updStmt, 2, statement->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(updStmt);
    sqlite3_finalize(updStmt);
}

void SqliteStatementRepository::upsertStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;
    if (statement->id.empty()) { addStatement(statement); return; }
    updateStatement(statement);
    if (pimpl_ && pimpl_->db && sqlite3_changes(pimpl_->db->handle()) == 0) {
        addStatement(statement);
    }
}

void SqliteStatementRepository::clearStatements() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM statements;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
