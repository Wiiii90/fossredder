#include "persistence/repositories/SqliteTransactionRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "core/models/Transaction.h"
#include <map>
#include <string>

using json = nlohmann::json;

struct SqliteTransactionRepository::Impl { sqlite3* db = nullptr; };

SqliteTransactionRepository::SqliteTransactionRepository(const std::string& dbPath) : pimpl_(std::make_unique<Impl>()) {
    if (sqlite3_open(dbPath.c_str(), &pimpl_->db) != SQLITE_OK) throw std::runtime_error("Failed to open sqlite db");
    const char* sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS transactions (id INTEGER PRIMARY KEY AUTOINCREMENT, booking_group_id INTEGER NOT NULL, booking_date TEXT, value_date TEXT, amount REAL, actor TEXT, metadata TEXT);";
    char* err = nullptr; if (sqlite3_exec(pimpl_->db, sql, nullptr, nullptr, &err) != SQLITE_OK) { if (err) sqlite3_free(err); }
}

SqliteTransactionRepository::~SqliteTransactionRepository(){ if (pimpl_ && pimpl_->db) sqlite3_close(pimpl_->db); }

static long long toLL(const std::string& s) {
    try { return std::stoll(s); } catch (...) { return -1; }
}

void SqliteTransactionRepository::addTransaction(const std::shared_ptr<Transaction>& transaction) {
    if (!transaction) return;
    auto it = transaction->metadata.find("booking_group_id");
    if (it == transaction->metadata.end()) return;
    long long bgId = toLL(it->second);
    if (bgId <= 0) return;

    const char* insSql = "INSERT INTO transactions (booking_group_id, booking_date, value_date, amount, actor, metadata) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, insSql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int64(stmt, 1, bgId);
    sqlite3_bind_text(stmt, 2, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, transaction->valueDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, transaction->amount);
    sqlite3_bind_text(stmt, 5, transaction->actor.c_str(), -1, SQLITE_TRANSIENT);
    json j = json::object(); for (const auto& kv : transaction->metadata) j[kv.first] = kv.second;
    std::string meta = j.dump();
    sqlite3_bind_text(stmt, 6, meta.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        long long id = sqlite3_last_insert_rowid(pimpl_->db);
        // store db id into metadata
        transaction->metadata["db_id"] = std::to_string(id);
    }
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactions() const {
    std::vector<std::shared_ptr<Transaction>> out;
    const char* sql = "SELECT id, booking_group_id, booking_date, value_date, amount, actor, metadata FROM transactions ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        long long bgId = sqlite3_column_int64(stmt, 1);
        const unsigned char* bdate = sqlite3_column_text(stmt, 2);
        const unsigned char* vdate = sqlite3_column_text(stmt, 3);
        double amount = sqlite3_column_double(stmt, 4);
        const unsigned char* actor = sqlite3_column_text(stmt, 5);
        const unsigned char* meta = sqlite3_column_text(stmt, 6);

        std::map<std::string, std::string> metaMap;
        try {
            if (meta) {
                json mj = json::parse(reinterpret_cast<const char*>(meta));
                for (json::iterator it = mj.begin(); it != mj.end(); ++it) metaMap[it.key()] = it.value().get<std::string>();
            }
        } catch (...) { metaMap.clear(); }
        metaMap["db_id"] = std::to_string(id);
        metaMap["booking_group_id"] = std::to_string(bgId);

        auto tx = std::make_shared<Transaction>(
            bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
            vdate ? reinterpret_cast<const char*>(vdate) : std::string(),
            amount,
            actor ? reinterpret_cast<const char*>(actor) : std::string(),
            std::move(metaMap)
        );
        out.push_back(tx);
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Transaction>> SqliteTransactionRepository::getTransactionById(const std::string& id) const {
    long long tid = toLL(id);
    if (tid <= 0) return std::nullopt;
    const char* sql = "SELECT id, booking_group_id, booking_date, value_date, amount, actor, metadata FROM transactions WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, tid);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }
    long long idv = sqlite3_column_int64(stmt, 0);
    long long bgId = sqlite3_column_int64(stmt, 1);
    const unsigned char* bdate = sqlite3_column_text(stmt, 2);
    const unsigned char* vdate = sqlite3_column_text(stmt, 3);
    double amount = sqlite3_column_double(stmt, 4);
    const unsigned char* actor = sqlite3_column_text(stmt, 5);
    const unsigned char* meta = sqlite3_column_text(stmt, 6);

    std::map<std::string, std::string> metaMap;
    try {
        if (meta) {
            json mj = json::parse(reinterpret_cast<const char*>(meta));
            for (json::iterator it = mj.begin(); it != mj.end(); ++it) metaMap[it.key()] = it.value().get<std::string>();
        }
    } catch (...) { metaMap.clear(); }
    metaMap["db_id"] = std::to_string(idv);
    metaMap["booking_group_id"] = std::to_string(bgId);

    auto tx = std::make_shared<Transaction>(
        bdate ? reinterpret_cast<const char*>(bdate) : std::string(),
        vdate ? reinterpret_cast<const char*>(vdate) : std::string(),
        amount,
        actor ? reinterpret_cast<const char*>(actor) : std::string(),
        std::move(metaMap)
    );
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
    auto it = transaction->metadata.find("db_id");
    if (it == transaction->metadata.end()) return;
    long long tid = toLL(it->second);
    if (tid <= 0) return;

    const char* sql = "UPDATE transactions SET booking_date = ?, value_date = ?, amount = ?, actor = ?, metadata = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, transaction->bookingDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, transaction->valueDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, transaction->amount);
    sqlite3_bind_text(stmt, 4, transaction->actor.c_str(), -1, SQLITE_TRANSIENT);
    json j = json::object(); for (const auto& kv : transaction->metadata) j[kv.first] = kv.second;
    std::string meta = j.dump();
    sqlite3_bind_text(stmt, 5, meta.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, tid);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
