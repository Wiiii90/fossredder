#include "persistence/SqliteDb.h"
#include "persistence/Uuid.h"
#include "persistence/repositories/SqliteAnnualRepository.h"
#include "core/models/Annual.h"
#include <sqlite3.h>
#include <stdexcept>
#include <string>

// Minimal functions for serializing vector<string> into a JSON-like
// array string. Keep it simple to avoid adding a JSON dependency.
static std::string joinStringArray(const std::vector<std::string>& v) {
    std::string out = "[";
    bool first = true;
    for (const auto& s : v) {
        if (!first) out += ",";
        out += '"';
        for (char c : s) {
            if (c == '"') out += '\\"'; else out += c;
        }
        out += '"';
        first = false;
    }
    out += "]";
    return out;
}

// Parsing is minimal and forgiving: expects simple quoted strings separated by commas.
static std::vector<std::string> parseStringArray(const std::string& s) {
    std::vector<std::string> out;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '"') {
            ++i;
            std::string cur;
            while (i < s.size() && s[i] != '"') {
                if (s[i] == '\\' && i + 1 < s.size()) { ++i; cur += s[i]; } else { cur += s[i]; }
                ++i;
            }
            if (i < s.size() && s[i] == '"') ++i;
            out.push_back(cur);
        } else ++i;
    }
    return out;
}

struct SqliteAnnualRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteAnnualRepository::SqliteAnnualRepository(const std::string& dbPath)
    : SqliteAnnualRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteAnnualRepository::SqliteAnnualRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteAnnualRepository::~SqliteAnnualRepository() = default;

// CRUD implementations
void SqliteAnnualRepository::addAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual) return;
    if (annual->id.empty()) annual->id = persistence::generateUuid();
    const char* ins = "INSERT INTO annuals (id, year, transaction_ids, assigned_analysis_ids, verification_state, created_at, updated_at, schema_version) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), ins, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,annual->id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2,annual->year);
    std::string txs = joinStringArray(annual->transactionIds);
    std::string aids = joinStringArray(annual->assignedAnalysisIds);
    sqlite3_bind_text(stmt,3,txs.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,aids.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,5,static_cast<int>(annual->verificationState));
    sqlite3_bind_text(stmt,6,annual->createdAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,7,annual->updatedAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,8,annual->schemaVersion);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnuals() const {
    std::vector<std::shared_ptr<Annual>> out;
    const char* sql = "SELECT id, year, transaction_ids, assigned_analysis_ids, verification_state, created_at, updated_at, schema_version FROM annuals ORDER BY year;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt,0);
        int year = sqlite3_column_int(stmt,1);
        const unsigned char* txstxt = sqlite3_column_text(stmt,2);
        const unsigned char* aidtxt = sqlite3_column_text(stmt,3);
        int vs = sqlite3_column_int(stmt,4);
        const unsigned char* catxt = sqlite3_column_text(stmt,5);
        const unsigned char* uptxt = sqlite3_column_text(stmt,6);
        int sv = sqlite3_column_int(stmt,7);

        auto a = std::make_shared<Annual>();
        a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        a->year = year;
        a->transactionIds = txstxt ? parseStringArray(reinterpret_cast<const char*>(txstxt)) : std::vector<std::string>{};
        a->assignedAnalysisIds = aidtxt ? parseStringArray(reinterpret_cast<const char*>(aidtxt)) : std::vector<std::string>{};
        a->verificationState = static_cast<Annual::VerificationState>(vs);
        a->createdAt = catxt ? reinterpret_cast<const char*>(catxt) : std::string();
        a->updatedAt = uptxt ? reinterpret_cast<const char*>(uptxt) : std::string();
        a->schemaVersion = sv;
        out.push_back(std::move(a));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Annual>> SqliteAnnualRepository::getAnnualById(const std::string& id) const {
    const char* sql = "SELECT id, year, transaction_ids, assigned_analysis_ids, verification_state, created_at, updated_at, schema_version FROM annuals WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }
    const unsigned char* idtxt = sqlite3_column_text(stmt,0);
    int year = sqlite3_column_int(stmt,1);
    const unsigned char* txstxt = sqlite3_column_text(stmt,2);
    const unsigned char* aidtxt = sqlite3_column_text(stmt,3);
    int vs = sqlite3_column_int(stmt,4);
    const unsigned char* catxt = sqlite3_column_text(stmt,5);
    const unsigned char* uptxt = sqlite3_column_text(stmt,6);
    int sv = sqlite3_column_int(stmt,7);

    auto a = std::make_shared<Annual>();
    a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    a->year = year;
    a->transactionIds = txstxt ? parseStringArray(reinterpret_cast<const char*>(txstxt)) : std::vector<std::string>{};
    a->assignedAnalysisIds = aidtxt ? parseStringArray(reinterpret_cast<const char*>(aidtxt)) : std::vector<std::string>{};
    a->verificationState = static_cast<Annual::VerificationState>(vs);
    a->createdAt = catxt ? reinterpret_cast<const char*>(catxt) : std::string();
    a->updatedAt = uptxt ? reinterpret_cast<const char*>(uptxt) : std::string();
    a->schemaVersion = sv;
    sqlite3_finalize(stmt);
    return std::make_optional<std::shared_ptr<Annual>>(a);
}

void SqliteAnnualRepository::removeAnnual(const std::string& id) {
    const char* sql = "DELETE FROM annuals WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteAnnualRepository::updateAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual) return;
    const char* sql = "UPDATE annuals SET year = ?, transaction_ids = ?, assigned_analysis_ids = ?, verification_state = ?, created_at = ?, updated_at = ?, schema_version = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int(stmt,1,annual->year);
    std::string txs = joinStringArray(annual->transactionIds);
    std::string aids = joinStringArray(annual->assignedAnalysisIds);
    sqlite3_bind_text(stmt,2,txs.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,aids.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,4,static_cast<int>(annual->verificationState));
    sqlite3_bind_text(stmt,5,annual->createdAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,6,annual->updatedAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,7,annual->schemaVersion);
    sqlite3_bind_text(stmt,8,annual->id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteAnnualRepository::upsertAnnual(const std::shared_ptr<Annual>& annual) {
    if (!annual) return;
    updateAnnual(annual);
    if (!getAnnualById(annual->id).has_value()) addAnnual(annual);
}

void SqliteAnnualRepository::clearAnnuals() {
    const char* sql = "DELETE FROM annuals;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

sqlite3* SqliteAnnualRepository::sqliteHandle() const noexcept { return pimpl_->db->handle(); }
