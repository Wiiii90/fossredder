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

    fprintf(stderr, "SqliteStatementRepository::addStatement: inserted statement id=%lld name='%s' transactions=%zu\n", statementId, statement->name.c_str(), statement->transactions.size());

    const char* insTxSql = "INSERT INTO transactions (statement_id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, metadata, proof_image_path, allocatable) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insTxSql, -1, &txStmt, nullptr) != SQLITE_OK) {
        return;
    }

    // prepare relation insert for transaction_properties
    const char* relSql = "INSERT OR IGNORE INTO transaction_properties (transaction_id, property_id) VALUES (?, ?);";
    sqlite3_stmt* relStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) != SQLITE_OK) {
        relStmt = nullptr; // continue without relations if prepare failed
    }

    size_t inserted = 0;
    for (Transaction& tx : statement->transactions) {
        sqlite3_reset(txStmt);
        sqlite3_bind_int64(txStmt, 1, statementId);
        sqlite3_bind_text(txStmt, 2, tx.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 3, tx.bookingDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 4, tx.valuta.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(txStmt, 5, tx.amount);
        sqlite3_bind_int(txStmt, 6, static_cast<int>(tx.status));
        sqlite3_bind_text(txStmt, 7, tx.description.c_str(), -1, SQLITE_TRANSIENT);

        long long actorId = (!tx.actorId.empty()) ? parseId(tx.actorId) : -1;
        long long contractId = (!tx.contractId.empty()) ? parseId(tx.contractId) : -1;
        if (actorId > 0) sqlite3_bind_int64(txStmt, 8, actorId); else sqlite3_bind_null(txStmt, 8);
        if (contractId > 0) sqlite3_bind_int64(txStmt, 9, contractId); else sqlite3_bind_null(txStmt, 9);

        sqlite3_bind_text(txStmt, 10, tx.metadata.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 11, tx.proofImagePath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(txStmt, 12, tx.allocatable ? 1 : 0);

        if (sqlite3_step(txStmt) != SQLITE_DONE) {
            // if transaction insert fails, skip relations and continue
            continue;
        }
        tx.id = std::to_string(lastInsertRowId(pimpl_->db->handle()));
        ++inserted;

        // insert transaction_properties relations if prepared
        if (relStmt) {
            for (const auto& pidStr : tx.propertyIds) {
                try {
                    long long pid = resolvePropertyId(pimpl_->db->handle(), pidStr);
                    if (pid <= 0) {
                        fprintf(stderr, "SqliteStatementRepository: unresolved property '%s' for transaction %s\n", pidStr.c_str(), tx.id.c_str());
                        continue;
                    }
                    sqlite3_reset(relStmt);
                    sqlite3_bind_int64(relStmt, 1, std::stoll(tx.id));
                    sqlite3_bind_int64(relStmt, 2, pid);
                    int rc = sqlite3_step(relStmt);
                    if (rc != SQLITE_DONE && rc != SQLITE_CONSTRAINT) {
                        const char* err = sqlite3_errmsg(pimpl_->db->handle());
                        fprintf(stderr, "SqliteStatementRepository: failed to insert transaction_property (tx=%s, prop=%s): %s\n", tx.id.c_str(), pidStr.c_str(), err ? err : "unknown");
                    }
                } catch (...) {
                    // ignore invalid property id strings
                }
            }
        }
    }

    fprintf(stderr, "SqliteStatementRepository::addStatement: inserted %zu transactions for statement %lld\n", inserted, statementId);

    if (txStmt) sqlite3_finalize(txStmt);
    if (relStmt) sqlite3_finalize(relStmt);
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

        const char* selTxSql = "SELECT id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, metadata, proof_image_path, allocatable FROM transactions WHERE statement_id = ? ORDER BY id;";
        sqlite3_stmt* txStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), selTxSql, -1, &txStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(txStmt, 1, statementId);
            while (sqlite3_step(txStmt) == SQLITE_ROW) {
                long long txId = sqlite3_column_int64(txStmt, 0);
                const unsigned char* txName = sqlite3_column_text(txStmt, 1);
                const unsigned char* bdate = sqlite3_column_text(txStmt, 2);
                const unsigned char* valuta = sqlite3_column_text(txStmt, 3);
                double amount = sqlite3_column_double(txStmt, 4);
                int status = sqlite3_column_int(txStmt, 5);
                const unsigned char* desc = sqlite3_column_text(txStmt, 6);
                long long actorId = sqlite3_column_type(txStmt, 7) == SQLITE_NULL ? -1 : sqlite3_column_int64(txStmt, 7);
                long long contractId = sqlite3_column_type(txStmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(txStmt, 8);
                const unsigned char* meta = sqlite3_column_text(txStmt, 9);
                const unsigned char* proof = sqlite3_column_text(txStmt, 10);
                int alloc = sqlite3_column_int(txStmt, 11);

                Transaction tx(
                    txName ? reinterpret_cast<const char*>(txName) : std::string(),
                    bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
                    valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
                    amount,
                    nullptr,
                    nullptr,
                    desc ? reinterpret_cast<const char*>(desc) : std::string(),
                    alloc != 0
                );
                tx.id = std::to_string(txId);
                tx.status = static_cast<Transaction::Status>(status);
                if (actorId > 0) tx.actorId = std::to_string(actorId);
                if (contractId > 0) tx.contractId = std::to_string(contractId);
                tx.metadata = meta ? reinterpret_cast<const char*>(meta) : std::string();
                tx.proofImagePath = proof ? reinterpret_cast<const char*>(proof) : std::string();
                // load properties for this transaction
                const char* relSql = "SELECT property_id FROM transaction_properties WHERE transaction_id = ? ORDER BY property_id;";
                sqlite3_stmt* relStmt = nullptr;
                if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_int64(relStmt, 1, txId);
                    while (sqlite3_step(relStmt) == SQLITE_ROW) {
                        long long pid = sqlite3_column_int64(relStmt, 0);
                        if (pid > 0) tx.propertyIds.push_back(std::to_string(pid));
                    }
                    sqlite3_finalize(relStmt);
                }
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

    const char* selTxSql = "SELECT id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, metadata, proof_image_path, allocatable FROM transactions WHERE statement_id = ? ORDER BY id;";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), selTxSql, -1, &txStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(txStmt, 1, statementId);
        while (sqlite3_step(txStmt) == SQLITE_ROW) {
            long long txId = sqlite3_column_int64(txStmt, 0);
            const unsigned char* txName = sqlite3_column_text(txStmt, 1);
            const unsigned char* bdate = sqlite3_column_text(txStmt, 2);
            const unsigned char* valuta = sqlite3_column_text(txStmt, 3);
            double amount = sqlite3_column_double(txStmt, 4);
            int status = sqlite3_column_int(txStmt, 5);
            const unsigned char* desc = sqlite3_column_text(txStmt, 6);
            long long actorId = sqlite3_column_type(txStmt, 7) == SQLITE_NULL ? -1 : sqlite3_column_int64(txStmt, 7);
            long long contractId = sqlite3_column_type(txStmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(txStmt, 8);
            const unsigned char* meta = sqlite3_column_text(txStmt, 9);
            const unsigned char* proof = sqlite3_column_text(txStmt, 10);
            int alloc = sqlite3_column_int(txStmt, 11);

            Transaction tx(
                txName ? reinterpret_cast<const char*>(txName) : std::string(),
                bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
                valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
                amount,
                nullptr,
                nullptr,
                desc ? reinterpret_cast<const char*>(desc) : std::string(),
                alloc != 0
            );
            tx.id = std::to_string(txId);
            tx.status = static_cast<Transaction::Status>(status);
            if (actorId > 0) tx.actorId = std::to_string(actorId);
            if (contractId > 0) tx.contractId = std::to_string(contractId);
            tx.metadata = meta ? reinterpret_cast<const char*>(meta) : std::string();
            tx.proofImagePath = proof ? reinterpret_cast<const char*>(proof) : std::string();
            // load properties for this transaction
            const char* relSql = "SELECT property_id FROM transaction_properties WHERE transaction_id = ? ORDER BY property_id;";
            sqlite3_stmt* relStmt = nullptr;
            if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int64(relStmt, 1, txId);
                while (sqlite3_step(relStmt) == SQLITE_ROW) {
                    long long pid = sqlite3_column_int64(relStmt, 0);
                    if (pid > 0) tx.propertyIds.push_back(std::to_string(pid));
                }
                sqlite3_finalize(relStmt);
            }
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

    const char* updStmtSql = "UPDATE statements SET name = ? WHERE id = ?;";
    sqlite3_stmt* updStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), updStmtSql, -1, &updStmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(updStmt, 1, statement->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(updStmt, 2, statementId);
    sqlite3_step(updStmt);
    sqlite3_finalize(updStmt);

    // Remove existing transactions for this statement and re-insert from the statement object
    const char* delTxSql = "DELETE FROM transactions WHERE statement_id = ?;";
    sqlite3_stmt* delStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), delTxSql, -1, &delStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(delStmt, 1, statementId);
        sqlite3_step(delStmt);
        sqlite3_finalize(delStmt);
    }

    const char* insTxSql = "INSERT INTO transactions (statement_id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, metadata, proof_image_path, allocatable) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* txStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insTxSql, -1, &txStmt, nullptr) != SQLITE_OK) txStmt = nullptr;

    // prepare relation insert for transaction_properties
    const char* relSql = "INSERT OR IGNORE INTO transaction_properties (transaction_id, property_id) VALUES (?, ?);";
    sqlite3_stmt* relStmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) != SQLITE_OK) relStmt = nullptr;

    for (Transaction& tx : statement->transactions) {
        if (!txStmt) break;
        sqlite3_reset(txStmt);
        sqlite3_bind_int64(txStmt, 1, statementId);
        sqlite3_bind_text(txStmt, 2, tx.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 3, tx.bookingDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 4, tx.valuta.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(txStmt, 5, tx.amount);
        sqlite3_bind_int(txStmt, 6, static_cast<int>(tx.status));
        sqlite3_bind_text(txStmt, 7, tx.description.c_str(), -1, SQLITE_TRANSIENT);

        long long actorId = (!tx.actorId.empty()) ? parseId(tx.actorId) : -1;
        long long contractId = (!tx.contractId.empty()) ? parseId(tx.contractId) : -1;
        if (actorId > 0) sqlite3_bind_int64(txStmt, 8, actorId); else sqlite3_bind_null(txStmt, 8);
        if (contractId > 0) sqlite3_bind_int64(txStmt, 9, contractId); else sqlite3_bind_null(txStmt, 9);

        sqlite3_bind_text(txStmt, 10, tx.metadata.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(txStmt, 11, tx.proofImagePath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(txStmt, 12, tx.allocatable ? 1 : 0);

        if (sqlite3_step(txStmt) != SQLITE_DONE) continue;
        tx.id = std::to_string(lastInsertRowId(pimpl_->db->handle()));

        if (relStmt) {
            for (const auto& pidStr : tx.propertyIds) {
                try {
                    long long pid = resolvePropertyId(pimpl_->db->handle(), pidStr);
                    if (pid <= 0) {
                        fprintf(stderr, "SqliteStatementRepository: unresolved property '%s' for transaction %s\n", pidStr.c_str(), tx.id.c_str());
                        continue;
                    }
                    sqlite3_reset(relStmt);
                    sqlite3_bind_int64(relStmt, 1, std::stoll(tx.id));
                    sqlite3_bind_int64(relStmt, 2, pid);
                    sqlite3_step(relStmt);
                } catch (...) {
                    // ignore invalid property id strings
                }
            }
        }
    }

    fprintf(stderr, "SqliteStatementRepository::updateStatement: reinserted %zu transactions for statement %s\n", statement->transactions.size(), statement->id.c_str());

    if (txStmt) sqlite3_finalize(txStmt);
    if (relStmt) sqlite3_finalize(relStmt);
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
