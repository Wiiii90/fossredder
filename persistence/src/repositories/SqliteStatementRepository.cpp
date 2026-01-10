#include "persistence/repositories/SqliteStatementRepository.h"
#include <sqlite3.h>
#include <stdexcept>
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

static long long lastInsertRowId(sqlite3* db) {
    return sqlite3_last_insert_rowid(db);
}

static long long parseId(const std::string& s) {
    try {
        size_t pos = 0;
        long long v = std::stoll(s, &pos);
        if (pos == s.size()) return v;
    } catch (...) {}
    return -1;
}

static long long resolvePropertyId(sqlite3* db, const std::string& pidStr) {
    // numeric string?
    try {
        size_t pos = 0;
        long long v = std::stoll(pidStr, &pos);
        if (pos == pidStr.size()) return v;
    } catch (...) {}

    const char* sel = "SELECT id FROM properties WHERE name = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sel, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
    long long out = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (out > 0) return out;

    // Not found - try to insert a minimal property row with this name
    const char* ins = "INSERT INTO properties (name, address, description, consumption, consumption_unit) VALUES (?, '', '', 0, '');";
    sqlite3_stmt* insStmt = nullptr;
    if (sqlite3_prepare_v2(db, ins, -1, &insStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(insStmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(insStmt) == SQLITE_DONE) {
            long long nid = sqlite3_last_insert_rowid(db);
            sqlite3_finalize(insStmt);
            if (nid > 0) return nid;
        } else {
            const char* err = sqlite3_errmsg(db);
            fprintf(stderr, "SqliteStatementRepository: failed to insert property '%s': %s\n", pidStr.c_str(), err ? err : "unknown");
        }
        sqlite3_finalize(insStmt);
    }

    // final attempt to lookup (in case of race)
    if (sqlite3_prepare_v2(db, sel, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) out = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
    }

    return out > 0 ? out : -1;
}

void SqliteStatementRepository::addStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;

    const char* insStmtSql = "INSERT INTO statements (name) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insStmtSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_text(stmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    long long statementId = lastInsertRowId(pimpl_->db->handle());
    statement->id = std::to_string(statementId);

    // Statement inserted; transactions are managed by the transaction repository
    fprintf(stderr, "SqliteStatementRepository::addStatement: inserted statement id=%lld name='%s'\n", statementId, statement->name.c_str());
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    const char* selStmtSql = "SELECT id, name FROM statements ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long statementId = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);

        auto s = std::make_shared<Statement>();
        s->id = std::to_string(statementId);
        s->name = name ? reinterpret_cast<const char*>(name) : std::string();

        out.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Statement>> SqliteStatementRepository::getStatementById(const std::string& id) const {
    long long sid = parseId(id);
    if (sid <= 0) return std::nullopt;

    const char* selStmtSql = "SELECT id, name FROM statements WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, sid);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    long long statementId = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    sqlite3_finalize(stmt);

    auto s = std::make_shared<Statement>();
    s->id = std::to_string(statementId);
    s->name = name ? reinterpret_cast<const char*>(name) : std::string();

    return s;
}

void SqliteStatementRepository::removeStatement(const std::string& id) {
    long long sid = parseId(id);
    if (sid <= 0) return;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM statements WHERE id = ?;";
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, sid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteStatementRepository::updateStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;
    long long statementId = parseId(statement->id);
    if (statementId <= 0) {
        addStatement(statement);
        return;
    }

    const char* updStmtSql = "UPDATE statements SET name = ? WHERE id = ?;";
    sqlite3_stmt* updStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), updStmtSql, -1, &updStmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(updStmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(updStmt, 2, statementId);
    sqlite3_step(updStmt);
    sqlite3_finalize(updStmt);
}

void SqliteStatementRepository::upsertStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;
    long long sid = parseId(statement->id);
    if (sid > 0) {
        updateStatement(statement);
        return;
    }
    addStatement(statement);
}

void SqliteStatementRepository::clearStatements() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM statements;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
