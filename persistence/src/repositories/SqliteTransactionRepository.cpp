#include "persistence/repositories/SqliteTransactionRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Transaction.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "persistence/SqliteDb.h"

struct SqliteTransactionRepository::Impl { std::shared_ptr<SqliteDb> db; };

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

// Resolve a property identifier string. If it's already numeric, return it.
// Otherwise, try to find the property id by name. Do NOT create new properties here.
static long long resolvePropertyId(sqlite3* db, const std::string& pidStr) {
    long long pid = toLL(pidStr);
    if (pid > 0) return pid;

    const char* sel = "SELECT id FROM properties WHERE name = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sel, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, pidStr.c_str(), -1, SQLITE_TRANSIENT);
    long long out = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return out > 0 ? out : -1;
}

static void clearRelations(sqlite3* db, long long transactionId) {
    const char* del = "DELETE FROM transaction_properties WHERE transaction_id = ?;";
    sqlite3_stmt* delStmt = nullptr;
    if (sqlite3_prepare_v2(db, del, -1, &delStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(delStmt, 1, transactionId);
        sqlite3_step(delStmt);
        sqlite3_finalize(delStmt);
    } else {
        const char* err = sqlite3_errmsg(db);
        fprintf(stderr, "SqliteTransactionRepository::clearRelations: prepare failed: %s\n", err ? err : "unknown");
    }
}

static void insertRelations(sqlite3* db, long long transactionId, const Transaction& t) {
    const char* sql = "INSERT OR IGNORE INTO transaction_properties (transaction_id, property_id) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        fprintf(stderr, "SqliteTransactionRepository::insertRelations: prepare failed: %s\n", err ? err : "unknown");
        return;
    }
    for (const auto& pidStr : t.propertyIds) {
        long long pid = resolvePropertyId(db, pidStr);
        if (pid <= 0) {
            fprintf(stderr, "SqliteTransactionRepository::insertRelations: unresolved property '%s' for transaction %lld\n", pidStr.c_str(), transactionId);
            continue;
        }
        sqlite3_reset(stmt);
        sqlite3_bind_int64(stmt, 1, transactionId);
        sqlite3_bind_int64(stmt, 2, pid);
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

    const char* insSql = "INSERT INTO transactions (name, booking_date, valuta, amount, status, description, type, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), insSql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_int(stmt, 5, static_cast<int>(transaction->status));
    sqlite3_bind_text(stmt, 6, transaction->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, transaction->type.c_str(), -1, SQLITE_TRANSIENT);

    long long actorId = (!transaction->actorId.empty()) ? toLL(transaction->actorId) : ((transaction->actor && !transaction->actor->id.empty()) ? toLL(transaction->actor->id) : -1);
    long long contractId = (!transaction->contractId.empty()) ? toLL(transaction->contractId) : ((transaction->contract && !transaction->contract->id.empty()) ? toLL(transaction->contract->id) : -1);
    long long statementId = (!transaction->statementId.empty()) ? toLL(transaction->statementId) : -1;

    if (actorId > 0) sqlite3_bind_int64(stmt, 8, actorId); else sqlite3_bind_null(stmt, 8);
    if (contractId > 0) sqlite3_bind_int64(stmt, 9, contractId); else sqlite3_bind_null(stmt, 9);
    if (statementId > 0) sqlite3_bind_int64(stmt, 10, statementId); else sqlite3_bind_null(stmt, 10);

    sqlite3_bind_text(stmt, 11, transaction->metadata.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, transaction->proofImagePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 13, transaction->allocatable ? 1 : 0);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        long long id = sqlite3_last_insert_rowid(pimpl_->db->handle());
        transaction->id = std::to_string(id);
        // Insert property relations for the newly created transaction
        insertRelations(pimpl_->db->handle(), id, *transaction);
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactions() const {
    std::vector<std::shared_ptr<Transaction>> out;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, status, description, type, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable FROM transactions ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* bdate = sqlite3_column_text(stmt, 2);
        const unsigned char* valuta = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        int status = sqlite3_column_int(stmt, 5);
        const unsigned char* desc = sqlite3_column_text(stmt, 6);
        const unsigned char* type = sqlite3_column_text(stmt, 7);
        long long actorId = sqlite3_column_type(stmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 8);
        long long contractId = sqlite3_column_type(stmt, 9) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 9);
        long long statementId = sqlite3_column_type(stmt, 10) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 10);

        const unsigned char* meta = sqlite3_column_text(stmt, 11);
        const unsigned char* proof = sqlite3_column_text(stmt, 12);
        int alloc = sqlite3_column_int(stmt, 13);

        auto tx = std::make_shared<Transaction>(
            name ? reinterpret_cast<const char*>(name) : std::string(),
            bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
            valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
            amount,
            nullptr,
            nullptr,
            desc ? reinterpret_cast<const char*>(desc) : std::string(),
            alloc != 0
        );
        tx->id = std::to_string(id);
        tx->status = static_cast<Transaction::Status>(status);
        if (type) tx->type = reinterpret_cast<const char*>(type);
        if (actorId > 0) tx->actorId = std::to_string(actorId);
        if (contractId > 0) tx->contractId = std::to_string(contractId);
        if (statementId > 0) tx->statementId = std::to_string(statementId);

        tx->metadata = meta ? reinterpret_cast<const char*>(meta) : std::string();
        tx->proofImagePath = proof ? reinterpret_cast<const char*>(proof) : std::string();

        out.push_back(std::move(tx));
    }
    sqlite3_finalize(stmt);

    // load properties for each transaction
    for (const auto& tptr : out) {
        const long long tid = toLL(tptr->id);
        const char* relSql = "SELECT property_id FROM transaction_properties WHERE transaction_id = ? ORDER BY property_id;";
        sqlite3_stmt* relStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), relSql, -1, &relStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(relStmt, 1, tid);
            while (sqlite3_step(relStmt) == SQLITE_ROW) {
                long long pid = sqlite3_column_int64(relStmt, 0);
                if (pid > 0) tptr->propertyIds.push_back(std::to_string(pid));
            }
            sqlite3_finalize(relStmt);
        }
    }

    return out;
}

std::optional<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionById(const std::string& id) const {
    long long tid = toLL(id);
    if (tid <= 0) return std::nullopt;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, status, description, type, actor_id, contract_id, statement_id, metadata, proof_image_path, allocatable FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, tid);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }

    long long rid = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* bdate = sqlite3_column_text(stmt, 2);
    const unsigned char* valuta = sqlite3_column_text(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    int status = sqlite3_column_int(stmt, 5);
    const unsigned char* desc = sqlite3_column_text(stmt, 6);
    const unsigned char* type = sqlite3_column_text(stmt, 7);
    long long actorId = sqlite3_column_type(stmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 8);
    long long contractId = sqlite3_column_type(stmt, 9) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 9);
    long long statementId = sqlite3_column_type(stmt, 10) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 10);

    const unsigned char* meta = sqlite3_column_text(stmt, 11);
    const unsigned char* proof = sqlite3_column_text(stmt, 12);
    int alloc = sqlite3_column_int(stmt, 13);

    auto tx = std::make_shared<Transaction>(
        name ? reinterpret_cast<const char*>(name) : std::string(),
        bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
        valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
        amount,
        nullptr,
        nullptr,
        desc ? reinterpret_cast<const char*>(desc) : std::string(),
        alloc != 0
    );
    tx->id = std::to_string(rid);
    tx->status = static_cast<Transaction::Status>(status);
    if (type) tx->type = reinterpret_cast<const char*>(type);
    if (actorId > 0) tx->actorId = std::to_string(actorId);
    if (contractId > 0) tx->contractId = std::to_string(contractId);
    if (statementId > 0) tx->statementId = std::to_string(statementId);

    tx->metadata = meta ? reinterpret_cast<const char*>(meta) : std::string();
    tx->proofImagePath = proof ? reinterpret_cast<const char*>(proof) : std::string();

    sqlite3_finalize(stmt);
    return tx;
}

void SqliteTransactionRepository::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    long long tid = toLL(transaction->id);
    if (tid <= 0) return;

    const char* sql = "UPDATE transactions SET name = ?, booking_date = ?, valuta = ?, amount = ?, status = ?, description = ?, type = ?, actor_id = ?, contract_id = ?, statement_id = ?, metadata = ?, proof_image_path = ?, allocatable = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_int(stmt, 5, static_cast<int>(transaction->status));
    sqlite3_bind_text(stmt, 6, transaction->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, transaction->type.c_str(), -1, SQLITE_TRANSIENT);

    long long actorId = (!transaction->actorId.empty()) ? toLL(transaction->actorId) : ((transaction->actor && !transaction->actor->id.empty()) ? toLL(transaction->actor->id) : -1);
    long long contractId = (!transaction->contractId.empty()) ? toLL(transaction->contractId) : ((transaction->contract && !transaction->contract->id.empty()) ? toLL(transaction->contract->id) : -1);
    long long statementId = (!transaction->statementId.empty()) ? toLL(transaction->statementId) : -1;

    if (actorId > 0) sqlite3_bind_int64(stmt, 8, actorId); else sqlite3_bind_null(stmt, 8);
    if (contractId > 0) sqlite3_bind_int64(stmt, 9, contractId); else sqlite3_bind_null(stmt, 9);
    if (statementId > 0) sqlite3_bind_int64(stmt, 10, statementId); else sqlite3_bind_null(stmt, 10);

    sqlite3_bind_text(stmt, 11, transaction->metadata.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, transaction->proofImagePath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 13, transaction->allocatable ? 1 : 0);

    sqlite3_bind_int64(stmt, 14, tid);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Clear existing relations then insert new ones
    clearRelations(pimpl_->db->handle(), tid);
    insertRelations(pimpl_->db->handle(), tid, *transaction);
}

void SqliteTransactionRepository::upsertTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    long long tid = toLL(transaction->id);
    if (tid > 0) {
        updateTransaction(transaction);
        return;
    }
    addTransaction(transaction);
}

void SqliteTransactionRepository::clearTransactions() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM transactions;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

void SqliteTransactionRepository::removeTransaction(const std::string& id) {
    long long tid = toLL(id);
    if (tid <= 0) return;
    const char* sql = "DELETE FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, tid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
