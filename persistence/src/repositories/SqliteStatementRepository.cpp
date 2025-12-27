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

void SqliteStatementRepository::addStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;

    const char* insStmtSql = "INSERT INTO statements (name, start_date, end_date) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insStmtSql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_text(stmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, statement->startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, statement->endDate.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    long long statementId = lastInsertRowId(pimpl_->db->handle());
    statement->id = std::to_string(statementId);

    const char* insTxSql = "INSERT INTO transactions (statement_id, name, booking_date, valuta, amount, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insTxSql, -1, &txStmt, nullptr) != SQLITE_OK) {
        return;
    }

    for (Transaction& tx : statement->transactions) {
        sqlite3_reset(txStmt);
        sqlite3_bind_int64(txStmt, 1, statementId);
        sqlite3_bind_text(txStmt, 2, tx.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 3, tx.bookingDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 4, tx.valuta.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(txStmt, 5, tx.amount);
        sqlite3_bind_text(txStmt, 6, tx.description.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(txStmt) != SQLITE_DONE) {
            sqlite3_finalize(txStmt);
            return;
        }
        tx.id = std::to_string(lastInsertRowId(pimpl_->db->handle()));
    }

    sqlite3_finalize(txStmt);
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    const char* selStmtSql = "SELECT id, name, start_date, end_date FROM statements ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long statementId = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* start = sqlite3_column_text(stmt, 2);
        const unsigned char* end = sqlite3_column_text(stmt, 3);

        auto s = std::make_shared<Statement>();
        s->id = std::to_string(statementId);
        s->name = name ? reinterpret_cast<const char*>(name) : std::string();
        s->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
        s->endDate = end ? reinterpret_cast<const char*>(end) : std::string();

        const char* selTxSql = "SELECT id, name, booking_date, valuta, amount, description FROM transactions WHERE statement_id = ? ORDER BY id;";
        sqlite3_stmt* txStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), selTxSql, -1, &txStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(txStmt, 1, statementId);
            while (sqlite3_step(txStmt) == SQLITE_ROW) {
                long long txId = sqlite3_column_int64(txStmt, 0);
                const unsigned char* txName = sqlite3_column_text(txStmt, 1);
                const unsigned char* bdate = sqlite3_column_text(txStmt, 2);
                const unsigned char* valuta = sqlite3_column_text(txStmt, 3);
                double amount = sqlite3_column_double(txStmt, 4);
                const unsigned char* desc = sqlite3_column_text(txStmt, 5);

                Transaction tx(
                    txName ? reinterpret_cast<const char*>(txName) : std::string(),
                    bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
                    valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
                    amount,
                    nullptr,
                    nullptr,
                    desc ? reinterpret_cast<const char*>(desc) : std::string()
                );
                tx.id = std::to_string(txId);
                s->transactions.push_back(std::move(tx));
            }
            sqlite3_finalize(txStmt);
        }

        out.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Statement>> SqliteStatementRepository::getStatementById(const std::string& id) const {
    long long sid = parseId(id);
    if (sid <= 0) return std::nullopt;

    const char* selStmtSql = "SELECT id, name, start_date, end_date FROM statements WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, sid);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    long long statementId = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* start = sqlite3_column_text(stmt, 2);
    const unsigned char* end = sqlite3_column_text(stmt, 3);
    sqlite3_finalize(stmt);

    auto s = std::make_shared<Statement>();
    s->id = std::to_string(statementId);
    s->name = name ? reinterpret_cast<const char*>(name) : std::string();
    s->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
    s->endDate = end ? reinterpret_cast<const char*>(end) : std::string();

    const char* selTxSql = "SELECT id, name, booking_date, valuta, amount, description FROM transactions WHERE statement_id = ? ORDER BY id;";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selTxSql, -1, &txStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(txStmt, 1, statementId);
        while (sqlite3_step(txStmt) == SQLITE_ROW) {
            long long txId = sqlite3_column_int64(txStmt, 0);
            const unsigned char* txName = sqlite3_column_text(txStmt, 1);
            const unsigned char* bdate = sqlite3_column_text(txStmt, 2);
            const unsigned char* valuta = sqlite3_column_text(txStmt, 3);
            double amount = sqlite3_column_double(txStmt, 4);
            const unsigned char* desc = sqlite3_column_text(txStmt, 5);

            Transaction tx(
                txName ? reinterpret_cast<const char*>(txName) : std::string(),
                bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
                valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
                amount,
                nullptr,
                nullptr,
                desc ? reinterpret_cast<const char*>(desc) : std::string()
            );
            tx.id = std::to_string(txId);
            s->transactions.push_back(std::move(tx));
        }
        sqlite3_finalize(txStmt);
    }

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

    const char* updStmtSql = "UPDATE statements SET name = ?, start_date = ?, end_date = ? WHERE id = ?;";
    sqlite3_stmt* updStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), updStmtSql, -1, &updStmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(updStmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updStmt, 2, statement->startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(updStmt, 3, statement->endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(updStmt, 4, statementId);
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
