#include "persistence/repositories/SqliteStatementRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "core/models/Statement.h"
#include "core/models/BookingGroup.h"
#include "core/models/Transaction.h"

using json = nlohmann::json;

struct SqliteStatementRepository::Impl { sqlite3* db = nullptr; };

SqliteStatementRepository::SqliteStatementRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sqlStatements = 
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS statements (id INTEGER PRIMARY KEY AUTOINCREMENT, sourceFile TEXT);"
        "CREATE TABLE IF NOT EXISTS booking_groups (id INTEGER PRIMARY KEY AUTOINCREMENT, statement_id INTEGER NOT NULL, title TEXT, FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE);"
        "CREATE TABLE IF NOT EXISTS transactions (id INTEGER PRIMARY KEY AUTOINCREMENT, booking_group_id INTEGER NOT NULL, booking_date TEXT, value_date TEXT, amount REAL, actor TEXT, metadata TEXT, FOREIGN KEY(booking_group_id) REFERENCES booking_groups(id) ON DELETE CASCADE);";
    char* err = nullptr; if (sqlite3_exec(pimpl_->db, sqlStatements, nullptr, nullptr, &err) != SQLITE_OK) { std::string e = err ? err : "unknown"; sqlite3_free(err); throw std::runtime_error(std::string("Failed to init schema: ") + e); }
}

SqliteStatementRepository::~SqliteStatementRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

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
    char* err = nullptr;

    // prepare variables
    const char* insBgSql = "INSERT INTO booking_groups (statement_id, title) VALUES (?, ?);";
    sqlite3_stmt* bgStmt = nullptr;
    const char* insTxSql = "INSERT INTO transactions (booking_group_id, booking_date, value_date, amount, actor, metadata) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* txStmt = nullptr;
    long long statementId = -1;

    // begin transaction
    if (sqlite3_exec(pimpl_->db, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return;
    }

    // insert statement
    const char* insStmtSql = "INSERT INTO statements (sourceFile) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, insStmtSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }
    sqlite3_bind_text(stmt, 1, statement->sourceFile.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }
    sqlite3_finalize(stmt);
    statementId = lastInsertRowId(pimpl_->db);

    // prepare booking group insert
    if (sqlite3_prepare_v2(pimpl_->db, insBgSql, -1, &bgStmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    // prepare transaction insert
    if (sqlite3_prepare_v2(pimpl_->db, insTxSql, -1, &txStmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(bgStmt);
        sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    // iterate booking groups
    for (const BookingGroup& bg : statement->bookingGroups) {
        sqlite3_reset(bgStmt);
        sqlite3_bind_int64(bgStmt, 1, statementId);
        sqlite3_bind_text(bgStmt, 2, bg.title.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(bgStmt) != SQLITE_DONE) {
            sqlite3_finalize(bgStmt);
            sqlite3_finalize(txStmt);
            sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
        }
        long long bgId = lastInsertRowId(pimpl_->db);

        // insert transactions
        for (const Transaction& tx : bg.transactions) {
            sqlite3_reset(txStmt);
            sqlite3_bind_int64(txStmt, 1, bgId);
            sqlite3_bind_text(txStmt, 2, tx.bookingDate.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(txStmt, 3, tx.valueDate.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(txStmt, 4, tx.amount);
            sqlite3_bind_text(txStmt, 5, tx.actor.c_str(), -1, SQLITE_TRANSIENT);
            // serialize metadata map to json
            json j = json::object();
            for (const auto& kv : tx.metadata) j[kv.first] = kv.second;
            std::string metaStr = j.dump();
            sqlite3_bind_text(txStmt, 6, metaStr.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(txStmt) != SQLITE_DONE) {
                sqlite3_finalize(bgStmt);
                sqlite3_finalize(txStmt);
                sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
                return;
            }
        }
    }

    sqlite3_finalize(bgStmt);
    sqlite3_finalize(txStmt);
    // commit
    if (sqlite3_exec(pimpl_->db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }
}

std::vector<std::shared_ptr<Statement>> SqliteStatementRepository::getStatements() const {
    std::vector<std::shared_ptr<Statement>> out;
    const char* selStmtSql = "SELECT id, sourceFile FROM statements ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, selStmtSql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long statementId = sqlite3_column_int64(stmt, 0);
        const unsigned char* src = sqlite3_column_text(stmt, 1);
        std::string sourceFile = src ? reinterpret_cast<const char*>(src) : std::string();

        auto s = std::make_shared<Statement>();
        s->sourceFile = sourceFile;

        // load booking groups for this statement
        const char* selBgSql = "SELECT id, title FROM booking_groups WHERE statement_id = ? ORDER BY id;";
        sqlite3_stmt* bgStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db, selBgSql, -1, &bgStmt, nullptr) != SQLITE_OK) continue;
        sqlite3_bind_int64(bgStmt, 1, statementId);
        while (sqlite3_step(bgStmt) == SQLITE_ROW) {
            long long bgId = sqlite3_column_int64(bgStmt, 0);
            const unsigned char* title = sqlite3_column_text(bgStmt, 1);
            BookingGroup bg;
            bg.title = title ? reinterpret_cast<const char*>(title) : std::string();

            // load transactions for this booking group
            const char* selTxSql = "SELECT id, booking_date, value_date, amount, actor, metadata FROM transactions WHERE booking_group_id = ? ORDER BY id;";
            sqlite3_stmt* txStmt = nullptr;
            if (sqlite3_prepare_v2(pimpl_->db, selTxSql, -1, &txStmt, nullptr) != SQLITE_OK) continue;
            sqlite3_bind_int64(txStmt, 1, bgId);
            while (sqlite3_step(txStmt) == SQLITE_ROW) {
                long long txId = sqlite3_column_int64(txStmt, 0);
                const unsigned char* bdate = sqlite3_column_text(txStmt, 1);
                const unsigned char* vdate = sqlite3_column_text(txStmt, 2);
                double amount = sqlite3_column_double(txStmt, 3);
                const unsigned char* actor = sqlite3_column_text(txStmt, 4);
                const unsigned char* meta = sqlite3_column_text(txStmt, 5);

                std::map<std::string, std::string> metaMap;
                try {
                    if (meta) {
                        json mj = json::parse(reinterpret_cast<const char*>(meta));
                        for (json::iterator it = mj.begin(); it != mj.end(); ++it) metaMap[it.key()] = it.value().get<std::string>();
                    }
                } catch (...) { metaMap.clear(); }
                // store db id into metadata for later reference
                metaMap["db_id"] = std::to_string(txId);
                metaMap["booking_group_id"] = std::to_string(bgId);

                Transaction tx(
                    bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
                    vdate ? reinterpret_cast<const char*>(vdate) : std::string(),
                    amount,
                    actor ? reinterpret_cast<const char*>(actor) : std::string(),
                    std::move(metaMap)
                );
                bg.transactions.push_back(std::move(tx));
            }
            sqlite3_finalize(txStmt);
            s->bookingGroups.push_back(std::move(bg));
        }
        sqlite3_finalize(bgStmt);
        out.push_back(s);
    }
    sqlite3_finalize(stmt);
    return out;
}

void SqliteStatementRepository::removeStatement(const std::string& id) {
    long long sid = parseId(id);
    sqlite3_stmt* stmt = nullptr;
    if (sid > 0) {
        const char* sql = "DELETE FROM statements WHERE id = ?;";
        if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
        sqlite3_bind_int64(stmt, 1, sid);
    } else {
        const char* sql = "DELETE FROM statements WHERE sourceFile = ?;";
        if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteStatementRepository::updateStatement(const std::shared_ptr<Statement>& statement) {
    if (!statement) return;
    // Find existing statement id by sourceFile
    const char* selSql = "SELECT id FROM statements WHERE sourceFile = ? LIMIT 1;";
    sqlite3_stmt* selStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, selSql, -1, &selStmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(selStmt, 1, statement->sourceFile.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(selStmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(selStmt);
        addStatement(statement);
        return;
    }
    long long statementId = sqlite3_column_int64(selStmt, 0);
    sqlite3_finalize(selStmt);

    char* err = nullptr;
    if (sqlite3_exec(pimpl_->db, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); return; }

    // delete existing booking_groups (transactions are cascade-deleted)
    const char* delBgSql = "DELETE FROM booking_groups WHERE statement_id = ?;";
    sqlite3_stmt* delStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, delBgSql, -1, &delStmt, nullptr) != SQLITE_OK) { sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }
    sqlite3_bind_int64(delStmt, 1, statementId);
    if (sqlite3_step(delStmt) != SQLITE_DONE) { sqlite3_finalize(delStmt); sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }
    sqlite3_finalize(delStmt);

    // re-insert booking groups and transactions similar to add
    const char* insBgSql = "INSERT INTO booking_groups (statement_id, title) VALUES (?, ?);";
    sqlite3_stmt* bgStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, insBgSql, -1, &bgStmt, nullptr) != SQLITE_OK) { sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }

    const char* insTxSql = "INSERT INTO transactions (booking_group_id, booking_date, value_date, amount, actor, metadata) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, insTxSql, -1, &txStmt, nullptr) != SQLITE_OK) { sqlite3_finalize(bgStmt); sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }

    for (const BookingGroup& bg : statement->bookingGroups) {
        sqlite3_reset(bgStmt);
        sqlite3_bind_int64(bgStmt, 1, statementId);
        sqlite3_bind_text(bgStmt, 2, bg.title.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(bgStmt) != SQLITE_DONE) { sqlite3_finalize(bgStmt); sqlite3_finalize(txStmt); sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }
        long long bgId = lastInsertRowId(pimpl_->db);

        for (const Transaction& tx : bg.transactions) {
            sqlite3_reset(txStmt);
            sqlite3_bind_int64(txStmt, 1, bgId);
            sqlite3_bind_text(txStmt, 2, tx.bookingDate.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(txStmt, 3, tx.valueDate.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(txStmt, 4, tx.amount);
            sqlite3_bind_text(txStmt, 5, tx.actor.c_str(), -1, SQLITE_TRANSIENT);
            json j = json::object();
            for (const auto& kv : tx.metadata) j[kv.first] = kv.second;
            std::string metaStr = j.dump();
            sqlite3_bind_text(txStmt, 6, metaStr.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(txStmt) != SQLITE_DONE) { sqlite3_finalize(bgStmt); sqlite3_finalize(txStmt); sqlite3_exec(pimpl_->db, "ROLLBACK;", nullptr, nullptr, nullptr); return; }
        }
    }

    sqlite3_finalize(bgStmt);
    sqlite3_finalize(txStmt);
    if (sqlite3_exec(pimpl_->db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); return; }
}
