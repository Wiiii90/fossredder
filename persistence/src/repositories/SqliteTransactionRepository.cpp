#include "persistence/repositories/SqliteTransactionRepository.h"
#include "core/errors/ErrorReporterRegistry.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Transaction.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "persistence/SqliteDb.h"
#include "persistence/Uuid.h"

struct SqliteTransactionRepository::Impl { std::shared_ptr<SqliteDb> db; };

// Helper: ensure property exists and return its id (string). If pidStr is
// already an id or a name, try to resolve; if missing, insert a new property
// row with generated id and the pidStr as name.
static std::string resolvePropertyIdText(sqlite3* db, const std::string& pidStr) {
    if (pidStr.empty()) return std::string();

    // Check if pidStr exists as id
    sqlite3_stmt* stmt = nullptr;
    const char* selId = "SELECT id FROM properties WHERE id = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, selId, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* out = sqlite3_column_text(stmt, 0);
            std::string res = out ? reinterpret_cast<const char*>(out) : std::string();
            sqlite3_finalize(stmt);
            return res;
        }
        sqlite3_finalize(stmt);
    }

    // Try to find by name
    const char* selName = "SELECT id FROM properties WHERE name = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, selName, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* out = sqlite3_column_text(stmt, 0);
            std::string res = out ? reinterpret_cast<const char*>(out) : std::string();
            sqlite3_finalize(stmt);
            return res;
        }
        sqlite3_finalize(stmt);
    }

    // Insert new property with generated id and name=pidStr
    const std::string newId = persistence::generateUuid();
    const char* ins = "INSERT INTO properties (id, name, address, description, consumption, consumption_unit) VALUES (?, ?, '', '', 0, '');";
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, newId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return newId;
        }
        sqlite3_finalize(stmt);
    }

    return std::string();
}

void SqliteTransactionRepository::assignTransactionsToContract(const std::string& contractId, const std::vector<std::string>& transactionIds) {
    // contractId may be empty -> clear assignments for given transactions
    sqlite3* db = pimpl_->db->handle();
    if (!db) return;

    // Begin transaction for atomicity
    char* err = nullptr;
    if (sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return;
    }

    const char* updSql = "UPDATE transactions SET contract_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, updSql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
        return;
    }

    for (const auto& tidStr : transactionIds) {
        if (tidStr.empty()) continue;
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (!contractId.empty()) sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 1);
        sqlite3_bind_text(stmt, 2, tidStr.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            // ignore individual errors but continue
        }
    }
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
    }
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionsForContract(const std::string& contractId) const {
    std::vector<std::shared_ptr<Transaction>> out;
    if (contractId.empty()) return out;

    const char* sql = "SELECT id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable FROM transactions WHERE contract_id = ? ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* bdate = sqlite3_column_text(stmt, 2);
        const unsigned char* valuta = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        const unsigned char* desc = sqlite3_column_text(stmt, 6);
        const unsigned char* actorTxt = sqlite3_column_text(stmt, 7);
        const unsigned char* contractTxt = sqlite3_column_text(stmt, 8);
        const unsigned char* statementTxt = sqlite3_column_text(stmt, 9);

        int alloc = sqlite3_column_int(stmt, 13);

        auto tx = std::make_shared<Transaction>(
            name ? reinterpret_cast<const char*>(name) : std::string(),
            bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
            valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
            amount,
            desc ? reinterpret_cast<const char*>(desc) : std::string(),
            alloc != 0
        );
        tx->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        tx->status = static_cast<Transaction::Status>(status);
        if (actorTxt) tx->actorId = reinterpret_cast<const char*>(actorTxt);
        if (contractTxt) tx->contractId = reinterpret_cast<const char*>(contractTxt);
        if (statementTxt) tx->statementId = reinterpret_cast<const char*>(statementTxt);
        out.push_back(std::move(tx));
    }
    sqlite3_finalize(stmt);

    // load properties for each transaction
    for (const auto& tptr : out) {
        const char* relSql = "SELECT property_id FROM transaction_properties WHERE transaction_id = ? ORDER BY property_id;";
        sqlite3_stmt* relStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(relStmt, 1, tptr->id.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(relStmt) == SQLITE_ROW) {
                const unsigned char* pidtxt = sqlite3_column_text(relStmt, 0);
                if (pidtxt) tptr->propertyIds.push_back(reinterpret_cast<const char*>(pidtxt));
            }
            sqlite3_finalize(relStmt);
        }
    }

    return out;
}

// Resolve a property identifier string. If it's already numeric, return it.
// Otherwise, try to find the property id by name. Do NOT create new properties here.
static long long resolvePropertyId(sqlite3* db, const std::string& pidStr) { return -1; }

static void clearRelations(sqlite3* db, const std::string& transactionId) {
    const char* del = "DELETE FROM transaction_properties WHERE transaction_id = ?;";
    sqlite3_stmt* delStmt = nullptr;
    if (sqlite3_prepare_v2(db, del, -1, &delStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(delStmt, 1, transactionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(delStmt);
        sqlite3_finalize(delStmt);
    } else {
        const char* err = sqlite3_errmsg(db);
        core::errors::report({
            core::errors::ErrorSeverity::Error,
            "persistence::SqliteTransactionRepository::clearRelations",
            std::string("prepare failed: ") + (err ? err : "unknown"),
            {}
        });
    }
}

static void insertRelations(sqlite3* db, const std::string& transactionId, const Transaction& t) {
    const char* sql = "INSERT OR IGNORE INTO transaction_properties (transaction_id, property_id) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        core::errors::report({
            core::errors::ErrorSeverity::Error,
            "persistence::SqliteTransactionRepository::insertRelations",
            std::string("prepare failed: ") + (err ? err : "unknown"),
            {}
        });
        return;
    }
    for (const auto& pidStr : t.propertyIds) {
        std::string pid = resolvePropertyIdText(db, pidStr);
        if (pid.empty()) {
            core::errors::report({
                core::errors::ErrorSeverity::Warning,
                "persistence::SqliteTransactionRepository::insertRelations",
                std::string("unresolved property '") + pidStr + "' for transaction '" + transactionId + "'",
                {}
            });
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, transactionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pid.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

SqliteTransactionRepository::SqliteTransactionRepository(const std::string& dbPath)
    : SqliteTransactionRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqliteTransactionRepository::SqliteTransactionRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteTransactionRepository::~SqliteTransactionRepository() = default;

void SqliteTransactionRepository::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    if (transaction->id.empty()) transaction->id = persistence::generateUuid();

    const char* insSql = "INSERT INTO transactions (id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insSql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, transaction->amount);
    sqlite3_bind_int(stmt, 6, static_cast<int>(transaction->status));
    sqlite3_bind_text(stmt, 7, transaction->description.c_str(), -1, SQLITE_TRANSIENT);

    if (!transaction->actorId.empty()) sqlite3_bind_text(stmt, 8, transaction->actorId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 8);
    if (!transaction->contractId.empty()) sqlite3_bind_text(stmt, 9, transaction->contractId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 9);
    if (!transaction->statementId.empty()) sqlite3_bind_text(stmt, 10, transaction->statementId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 10);

    sqlite3_bind_null(stmt, 11);
    sqlite3_bind_null(stmt, 12);
    sqlite3_bind_int(stmt, 13, transaction->allocatable ? 1 : 0);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        // Insert property relations for the newly created transaction
        insertRelations(pimpl_->db->handle(), transaction->id, *transaction);
        core::errors::report({
            core::errors::ErrorSeverity::Info,
            "persistence::SqliteTransactionRepository::addTransaction",
            std::string("inserted id='") + transaction->id + "' name='" + transaction->name + "'",
            {}
        });
    }
    else {
        const char* err = sqlite3_errmsg(pimpl_->db->handle());
        core::errors::report({
            core::errors::ErrorSeverity::Error,
            "persistence::SqliteTransactionRepository::addTransaction",
            std::string("failed to insert id='") + transaction->id + "' name='" + transaction->name + "' err=" + (err ? err : "unknown"),
            {}
        });
    }
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactions() const {
    std::vector<std::shared_ptr<Transaction>> out;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable FROM transactions ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* bdate = sqlite3_column_text(stmt, 2);
        const unsigned char* valuta = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        const unsigned char* desc = sqlite3_column_text(stmt, 6);
        const unsigned char* actorTxt = sqlite3_column_text(stmt, 7);
        const unsigned char* contractTxt = sqlite3_column_text(stmt, 8);
        const unsigned char* statementTxt = sqlite3_column_text(stmt, 9);

        int alloc = sqlite3_column_int(stmt, 13);

        auto tx = std::make_shared<Transaction>(
            name ? reinterpret_cast<const char*>(name) : std::string(),
            bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
            valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
            amount,
            desc ? reinterpret_cast<const char*>(desc) : std::string(),
            alloc != 0
        );
        tx->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        tx->status = static_cast<Transaction::Status>(status);
        if (actorTxt) tx->actorId = reinterpret_cast<const char*>(actorTxt);
        if (contractTxt) tx->contractId = reinterpret_cast<const char*>(contractTxt);
        if (statementTxt) tx->statementId = reinterpret_cast<const char*>(statementTxt);
        out.push_back(std::move(tx));
    }
    sqlite3_finalize(stmt);

    // load properties for each transaction
    for (const auto& tptr : out) {
        const char* relSql = "SELECT property_id FROM transaction_properties WHERE transaction_id = ? ORDER BY property_id;";
        sqlite3_stmt* relStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(relStmt, 1, tptr->id.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(relStmt) == SQLITE_ROW) {
                const unsigned char* pidtxt = sqlite3_column_text(relStmt, 0);
                if (pidtxt) tptr->propertyIds.push_back(reinterpret_cast<const char*>(pidtxt));
            }
            sqlite3_finalize(relStmt);
        }
    }

    return out;
}

std::optional<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionById(const std::string& id) const {
    if (id.empty()) return std::nullopt;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, status, description, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }

    const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* bdate = sqlite3_column_text(stmt, 2);
    const unsigned char* valuta = sqlite3_column_text(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    int status = sqlite3_column_int(stmt, 5);
    const unsigned char* desc = sqlite3_column_text(stmt, 6);
    const unsigned char* actorTxt = sqlite3_column_text(stmt, 7);
    const unsigned char* contractTxt = sqlite3_column_text(stmt, 8);
    const unsigned char* statementTxt = sqlite3_column_text(stmt, 9);

    int alloc = sqlite3_column_int(stmt, 13);

    auto tx = std::make_shared<Transaction>(
        name ? reinterpret_cast<const char*>(name) : std::string(),
        bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
        valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
        amount,
        desc ? reinterpret_cast<const char*>(desc) : std::string(),
        alloc != 0
    );
    tx->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    tx->status = static_cast<Transaction::Status>(status);
    if (actorTxt) tx->actorId = reinterpret_cast<const char*>(actorTxt);
    if (contractTxt) tx->contractId = reinterpret_cast<const char*>(contractTxt);
    if (statementTxt) tx->statementId = reinterpret_cast<const char*>(statementTxt);
    sqlite3_finalize(stmt);
    return tx;
}

void SqliteTransactionRepository::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    if (transaction->id.empty()) return;

    const char* sql = "UPDATE transactions SET name = ?, booking_date = ?, valuta = ?, amount = ?, status = ?, description = ?, actor_id = ?, contract_id = ?, statement_id = ?, metadata = ?, proof_image_path = ?, allocatable = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_int(stmt, 5, static_cast<int>(transaction->status));
    sqlite3_bind_text(stmt, 6, transaction->description.c_str(), -1, SQLITE_TRANSIENT);

    if (!transaction->actorId.empty()) sqlite3_bind_text(stmt, 7, transaction->actorId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 7);
    if (!transaction->contractId.empty()) sqlite3_bind_text(stmt, 8, transaction->contractId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 8);
    if (!transaction->statementId.empty()) sqlite3_bind_text(stmt, 9, transaction->statementId.c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 9);

    sqlite3_bind_null(stmt, 10);
    sqlite3_bind_null(stmt, 11);
    sqlite3_bind_int(stmt, 12, transaction->allocatable ? 1 : 0);

    sqlite3_bind_text(stmt, 13, transaction->id.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) {
        const char* err = sqlite3_errmsg(pimpl_->db->handle());
        core::errors::report({
            core::errors::ErrorSeverity::Warning,
            "persistence::SqliteTransactionRepository::updateTransaction",
            std::string("no rows updated for id='") + transaction->id + "' err=" + (err ? err : "unknown"),
            {}
        });
    }
    else {
        core::errors::report({
            core::errors::ErrorSeverity::Info,
            "persistence::SqliteTransactionRepository::updateTransaction",
            std::string("updated id='") + transaction->id + "' name='" + transaction->name + "'",
            {}
        });
    }
    sqlite3_finalize(stmt);

    // Clear existing relations then insert new ones
    clearRelations(pimpl_->db->handle(), transaction->id);
    insertRelations(pimpl_->db->handle(), transaction->id, *transaction);
}

void SqliteTransactionRepository::upsertTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    core::errors::report({
        core::errors::ErrorSeverity::Info,
        "persistence::SqliteTransactionRepository::upsertTransaction",
        std::string("upsert id='") + transaction->id + "' name='" + transaction->name + "'",
        {}
    });
    if (transaction->id.empty()) { addTransaction(transaction); return; }

    // Check existence first to avoid race where update reports no changes but
    // the row exists (causing subsequent insert attempts to fail with UNIQUE).
    bool exists = false;
    if (pimpl_ && pimpl_->db) {
        sqlite3_stmt* selStmt = nullptr;
        const char* sel = "SELECT 1 FROM transactions WHERE id = ? LIMIT 1;";
        if (sqlite3_prepare_v2(pimpl_->db->handle(), sel, -1, &selStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(selStmt, 1, transaction->id.c_str(), -1, SQLITE_TRANSIENT);
            exists = (sqlite3_step(selStmt) == SQLITE_ROW);
            sqlite3_finalize(selStmt);
        }
    }

    if (exists) {
        updateTransaction(transaction);
    } else {
        addTransaction(transaction);
    }
}

void SqliteTransactionRepository::clearTransactions() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM transactions;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

void SqliteTransactionRepository::removeTransaction(const std::string& id) {
    if (id.empty()) return;
    const char* sql = "DELETE FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
