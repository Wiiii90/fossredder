#include "persistence/repositories/SqliteTransactionRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include "core/models/Transaction.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"

struct SqliteTransactionRepository::Impl { sqlite3* db = nullptr; };

SqliteTransactionRepository::SqliteTransactionRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "booking_date TEXT,"
        "valuta TEXT,"
        "amount REAL,"
        "description TEXT,"
        "actor_id INTEGER,"
        "contract_id INTEGER,"
        "statement_id INTEGER,"
        "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE SET NULL,"
        "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE SET NULL,"
        "FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE"
        ");";
    char* err = nullptr;
    if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); }
}

SqliteTransactionRepository::~SqliteTransactionRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

void SqliteTransactionRepository::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;

    const char* insSql = "INSERT INTO transactions (name, booking_date, valuta, amount, description, actor_id, contract_id) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, insSql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_text(stmt, 5, transaction->description.c_str(), -1, SQLITE_TRANSIENT);

    long long actorId = (transaction->actor && !transaction->actor->id.empty()) ? toLL(transaction->actor->id) : -1;
    long long contractId = (transaction->contract && !transaction->contract->id.empty()) ? toLL(transaction->contract->id) : -1;

    if (actorId > 0) sqlite3_bind_int64(stmt, 6, actorId); else sqlite3_bind_null(stmt, 6);
    if (contractId > 0) sqlite3_bind_int64(stmt, 7, contractId); else sqlite3_bind_null(stmt, 7);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        long long id = sqlite3_last_insert_rowid(pimpl_->db);
        transaction->id = std::to_string(id);
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactions() const {
    std::vector<std::shared_ptr<Transaction>> out;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, description, actor_id, contract_id, statement_id FROM transactions ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* bdate = sqlite3_column_text(stmt, 2);
        const unsigned char* valuta = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char* desc = sqlite3_column_text(stmt, 5);
        long long actorId = sqlite3_column_type(stmt, 6) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 6);
        long long contractId = sqlite3_column_type(stmt, 7) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 7);
        long long statementId = sqlite3_column_type(stmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 8);

        auto tx = std::make_shared<Transaction>(
            name ? reinterpret_cast<const char*>(name) : std::string(),
            bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
            valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
            amount,
            nullptr,
            nullptr,
            desc ? reinterpret_cast<const char*>(desc) : std::string()
        );
        tx->id = std::to_string(id);
        if (actorId > 0) tx->actorId = std::to_string(actorId);
        if (contractId > 0) tx->contractId = std::to_string(contractId);
        if (statementId > 0) tx->statementId = std::to_string(statementId);
        out.push_back(std::move(tx));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionById(const std::string& id) const {
    long long tid = toLL(id);
    if (tid <= 0) return std::nullopt;
    const char* sql = "SELECT id, name, booking_date, valuta, amount, description, actor_id, contract_id, statement_id FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, tid);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }

    long long rid = sqlite3_column_int64(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* bdate = sqlite3_column_text(stmt, 2);
    const unsigned char* valuta = sqlite3_column_text(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    const unsigned char* desc = sqlite3_column_text(stmt, 5);
    long long actorId = sqlite3_column_type(stmt, 6) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 6);
    long long contractId = sqlite3_column_type(stmt, 7) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 7);
    long long statementId = sqlite3_column_type(stmt, 8) == SQLITE_NULL ? -1 : sqlite3_column_int64(stmt, 8);

    auto tx = std::make_shared<Transaction>(
        name ? reinterpret_cast<const char*>(name) : std::string(),
        bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
        valuta ? reinterpret_cast<const char*>(valuta) : std::string(),
        amount,
        nullptr,
        nullptr,
        desc ? reinterpret_cast<const char*>(desc) : std::string()
    );
    tx->id = std::to_string(rid);
    if (actorId > 0) tx->actorId = std::to_string(actorId);
    if (contractId > 0) tx->contractId = std::to_string(contractId);
    if (statementId > 0) tx->statementId = std::to_string(statementId);

    sqlite3_finalize(stmt);
    return tx;
}

void SqliteTransactionRepository::removeTransaction(const std::string& id) {
    long long tid = toLL(id);
    if (tid <= 0) return;
    const char* sql = "DELETE FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, tid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteTransactionRepository::updateTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    long long tid = toLL(transaction->id);
    if (tid <= 0) return;

    const char* sql = "UPDATE transactions SET name = ?, booking_date = ?, valuta = ?, amount = ?, description = ?, actor_id = ?, contract_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, transaction->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valuta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_text(stmt, 5, transaction->description.c_str(), -1, SQLITE_TRANSIENT);

    long long actorId = (transaction->actor && !transaction->actor->id.empty()) ? toLL(transaction->actor->id) : -1;
    long long contractId = (transaction->contract && !transaction->contract->id.empty()) ? toLL(transaction->contract->id) : -1;

    if (actorId > 0) sqlite3_bind_int64(stmt, 6, actorId); else sqlite3_bind_null(stmt, 6);
    if (contractId > 0) sqlite3_bind_int64(stmt, 7, contractId); else sqlite3_bind_null(stmt, 7);

    sqlite3_bind_int64(stmt, 8, tid);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
