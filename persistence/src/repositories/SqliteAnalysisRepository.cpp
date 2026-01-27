#include "persistence/repositories/SqliteAnalysisRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include "persistence/SqliteDb.h"
#include "persistence/Uuid.h"
#include "core/models/Analysis.h"

struct SqliteAnalysisRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteAnalysisRepository::SqliteAnalysisRepository(const std::string& dbPath)
    : SqliteAnalysisRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqliteAnalysisRepository::SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteAnalysisRepository::~SqliteAnalysisRepository() = default;

void SqliteAnalysisRepository::addAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis) return;
    if (analysis->id.empty()) analysis->id = persistence::generateUuid();

    const char* ins = "INSERT INTO analyses (id, name, type, config_json, filter_spec, created_at, updated_at, schema_version) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), ins, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,analysis->id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,analysis->name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,analysis->type.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,analysis->configJson.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,5,analysis->filterSpec.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,6,analysis->createdAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,7,analysis->updatedAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,8,analysis->schemaVersion);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalyses() const {
    std::vector<std::shared_ptr<Analysis>> out;
    const char* sql = "SELECT id, name, type, config_json, filter_spec, created_at, updated_at, schema_version FROM analyses ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt,0);
        const unsigned char* nametxt = sqlite3_column_text(stmt,1);
        const unsigned char* typetxt = sqlite3_column_text(stmt,2);
        const unsigned char* cfgtxt = sqlite3_column_text(stmt,3);
        const unsigned char* filtertxt = sqlite3_column_text(stmt,4);
        const unsigned char* catxt = sqlite3_column_text(stmt,5);
        const unsigned char* uptxt = sqlite3_column_text(stmt,6);
        int sv = sqlite3_column_int(stmt,7);

        auto a = std::make_shared<Analysis>();
        a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        a->name = nametxt ? reinterpret_cast<const char*>(nametxt) : std::string();
        a->type = typetxt ? reinterpret_cast<const char*>(typetxt) : std::string();
        a->configJson = cfgtxt ? reinterpret_cast<const char*>(cfgtxt) : std::string();
        a->filterSpec = filtertxt ? reinterpret_cast<const char*>(filtertxt) : std::string();
        a->createdAt = catxt ? reinterpret_cast<const char*>(catxt) : std::string();
        a->updatedAt = uptxt ? reinterpret_cast<const char*>(uptxt) : std::string();
        a->schemaVersion = sv;
        out.push_back(std::move(a));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalysisById(const std::string& id) const {
    const char* sql = "SELECT id, name, type, config_json, filter_spec, created_at, updated_at, schema_version FROM analyses WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) { sqlite3_finalize(stmt); return std::nullopt; }
    const unsigned char* idtxt = sqlite3_column_text(stmt,0);
    const unsigned char* nametxt = sqlite3_column_text(stmt,1);
    const unsigned char* typetxt = sqlite3_column_text(stmt,2);
    const unsigned char* cfgtxt = sqlite3_column_text(stmt,3);
    const unsigned char* filtertxt = sqlite3_column_text(stmt,4);
    const unsigned char* catxt = sqlite3_column_text(stmt,5);
    const unsigned char* uptxt = sqlite3_column_text(stmt,6);
    int sv = sqlite3_column_int(stmt,7);

    auto a = std::make_shared<Analysis>();
    a->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    a->name = nametxt ? reinterpret_cast<const char*>(nametxt) : std::string();
    a->type = typetxt ? reinterpret_cast<const char*>(typetxt) : std::string();
    a->configJson = cfgtxt ? reinterpret_cast<const char*>(cfgtxt) : std::string();
    a->filterSpec = filtertxt ? reinterpret_cast<const char*>(filtertxt) : std::string();
    a->createdAt = catxt ? reinterpret_cast<const char*>(catxt) : std::string();
    a->updatedAt = uptxt ? reinterpret_cast<const char*>(uptxt) : std::string();
    a->schemaVersion = sv;
    sqlite3_finalize(stmt);
    return std::make_optional<std::shared_ptr<Analysis>>(a);
}

void SqliteAnalysisRepository::removeAnalysis(const std::string& id) {
    const char* sql = "DELETE FROM analyses WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteAnalysisRepository::updateAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis) return;
    const char* sql = "UPDATE analyses SET name = ?, type = ?, config_json = ?, filter_spec = ?, created_at = ?, updated_at = ?, schema_version = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,analysis->name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2,analysis->type.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3,analysis->configJson.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,4,analysis->filterSpec.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,5,analysis->createdAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,6,analysis->updatedAt.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,7,analysis->schemaVersion);
    sqlite3_bind_text(stmt,8,analysis->id.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteAnalysisRepository::upsertAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis) return;
    // Try update first
    updateAnalysis(analysis);
    // Then insert if missing
    if (!getAnalysisById(analysis->id).has_value()) addAnalysis(analysis);
}

void SqliteAnalysisRepository::clearAnalyses() {
    const char* sql = "DELETE FROM analyses;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

sqlite3* SqliteAnalysisRepository::sqliteHandle() const noexcept { return pimpl_->db->handle(); }
