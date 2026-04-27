#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Analysis.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <unordered_set>

struct SqliteAnalysisRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteAnalysisRepository::SqliteAnalysisRepository(const std::string& dbPath)
    : SqliteAnalysisRepository(std::make_shared<SqliteDb>(dbPath)) {}

SqliteAnalysisRepository::SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteAnalysisRepository::~SqliteAnalysisRepository() = default;

namespace {
std::unordered_set<std::string> analysisColumns(sqlite3* db) {
    std::unordered_set<std::string> columns;
    if (!db) return columns;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA table_info(analyses);", -1, &stmt, nullptr) != SQLITE_OK)
        return columns;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto* columnName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (columnName) columns.insert(std::string(columnName));
    }
    sqlite3_finalize(stmt);
    return columns;
}

std::string makeSelectAnalysisSql(sqlite3* db) {
    const auto columns = analysisColumns(db);
    const bool hasExportFormat = columns.find("export_format") != columns.end();
    const bool hasIncludeCalcAdjustments = columns.find("include_calc_adjustments") != columns.end();
    const bool hasExportStateJson = columns.find("export_state_json") != columns.end();
    const bool hasSnapshotTransactionsJson = columns.find("snapshot_transactions_json") != columns.end();

    std::string sql = "SELECT id, name, type, config_json, filter_spec";
    sql += hasExportFormat ? ", export_format" : ", '' AS export_format";
    sql += hasIncludeCalcAdjustments ? ", include_calc_adjustments" : ", 1 AS include_calc_adjustments";
    sql += hasExportStateJson ? ", export_state_json" : ", '{}' AS export_state_json";
    sql += hasSnapshotTransactionsJson ? ", snapshot_transactions_json" : ", '{}' AS snapshot_transactions_json";
    sql += ", created_at, updated_at, schema_version FROM analyses";
    return sql;
}

std::shared_ptr<Analysis> readAnalysis(persistence::StmtGuard& s) {
    auto a = std::make_shared<Analysis>();
    a->id = s.columnText(0); a->name = s.columnText(1); a->type = s.columnText(2);
    a->configJson = s.columnText(3); a->filterSpec = s.columnText(4);
    a->exportFormat = s.columnText(5);
    a->includeCalcAdjustments = s.columnInt(6) != 0;
    a->exportStateJson = s.columnText(7);
    a->snapshotTransactionsJson = s.columnText(8);
    a->createdAt = s.columnText(9); a->updatedAt = s.columnText(10);
    a->schemaVersion = s.columnInt(11);
    return a;
}

}

void SqliteAnalysisRepository::addAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis || analysis->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO analyses (id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at, schema_version)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) return;
    stmt.bindText(1, analysis->id);   stmt.bindText(2, analysis->name);
    stmt.bindText(3, analysis->type); stmt.bindText(4, analysis->configJson);
    stmt.bindText(5, analysis->filterSpec);
    stmt.bindText(6, analysis->exportFormat);
    stmt.bindInt (7, analysis->includeCalcAdjustments ? 1 : 0);
    stmt.bindText(8, analysis->exportStateJson);
    stmt.bindText(9, analysis->snapshotTransactionsJson);
    stmt.bindText(10, analysis->createdAt);
    stmt.bindText(11, analysis->updatedAt);
    stmt.bindInt (12, analysis->schemaVersion);
    stmt.step();
}

std::vector<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalyses() const {
    std::vector<std::shared_ptr<Analysis>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        makeSelectAnalysisSql(pimpl_->db->handle()) + " ORDER BY id;");
    if (!stmt) return out;
    while (stmt.step() == SQLITE_ROW) out.push_back(readAnalysis(stmt));
    return out;
}

std::optional<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalysisById(const std::string& id) const {
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        makeSelectAnalysisSql(pimpl_->db->handle()) + " WHERE id = ? LIMIT 1;");
    if (!stmt) return std::nullopt;
    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) return std::nullopt;
    return readAnalysis(stmt);
}

void SqliteAnalysisRepository::removeAnalysis(const std::string& id) {
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM analyses WHERE id = ?;");
    if (!stmt) return;
    stmt.bindText(1, id); stmt.step();
}

void SqliteAnalysisRepository::updateAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis || analysis->id.empty()) return;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE analyses SET name=?, type=?, config_json=?, filter_spec=?, export_format=?, include_calc_adjustments=?, export_state_json=?, snapshot_transactions_json=?, created_at=?, updated_at=?, schema_version=?"
        " WHERE id=?;");
    if (!stmt) return;
    stmt.bindText(1, analysis->name);       stmt.bindText(2, analysis->type);
    stmt.bindText(3, analysis->configJson); stmt.bindText(4, analysis->filterSpec);
    stmt.bindText(5, analysis->exportFormat);
    stmt.bindInt (6, analysis->includeCalcAdjustments ? 1 : 0);
    stmt.bindText(7, analysis->exportStateJson);
    stmt.bindText(8, analysis->snapshotTransactionsJson);
    stmt.bindText(9, analysis->createdAt);
    stmt.bindText(10, analysis->updatedAt);
    stmt.bindInt (11, analysis->schemaVersion);
    stmt.bindText(12, analysis->id);
    stmt.step();
}

void SqliteAnalysisRepository::upsertAnalysis(const std::shared_ptr<Analysis>& analysis) {
    if (!analysis || analysis->id.empty()) return;
    updateAnalysis(analysis);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) addAnalysis(analysis);
}

void SqliteAnalysisRepository::clearAnalyses() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM analyses;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

sqlite3* SqliteAnalysisRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
