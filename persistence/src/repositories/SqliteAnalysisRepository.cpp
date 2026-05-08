/**
 * @file persistence/src/repositories/SqliteAnalysisRepository.cpp
 * @brief Implements the SQLite-backed analysis repository.
 */

#include "persistence/repositories/SqliteAnalysisRepository.h"
#include "persistence/StmtGuard.h"
#include "core/models/Analysis.h"
#include "persistence/SqliteDb.h"
#include <sqlite3.h>
#include <stdexcept>
#include <string>

struct SqliteAnalysisRepository::Impl {
    std::shared_ptr<SqliteDb> db;
};

SqliteAnalysisRepository::SqliteAnalysisRepository(const std::string& dbPath)
    : SqliteAnalysisRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteAnalysisRepository::SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>())
{
    if (!db) {
        throw std::runtime_error("db is null");
    }

    pimpl_->db = std::move(db);
}

SqliteAnalysisRepository::~SqliteAnalysisRepository() = default;

std::shared_ptr<Analysis> readAnalysis(persistence::StmtGuard& s)
{
    auto a = std::make_shared<Analysis>();
    a->id = s.columnText(0); a->name = s.columnText(1); a->type = s.columnText(2);
    a->configJson = s.columnText(3); a->filterSpec = s.columnText(4);
    a->exportFormat = s.columnText(5);
    a->includeCalcAdjustments = s.columnInt(6) != 0;
    a->exportStateJson = s.columnText(7);
    a->snapshotTransactionsJson = s.columnText(8);
    a->createdAt = s.columnText(9); a->updatedAt = s.columnText(10);
    return a;
}

void SqliteAnalysisRepository::addAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id.empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "INSERT OR IGNORE INTO analyses (id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, analysis->id);
    stmt.bindText(2, analysis->name);
    stmt.bindText(3, analysis->type);
    stmt.bindText(4, analysis->configJson);
    stmt.bindText(5, analysis->filterSpec);
    stmt.bindText(6, analysis->exportFormat);
    stmt.bindInt(7, analysis->includeCalcAdjustments ? 1 : 0);
    stmt.bindText(8, analysis->exportStateJson);
    stmt.bindText(9, analysis->snapshotTransactionsJson);
    stmt.bindText(10, analysis->createdAt);
    stmt.bindText(11, analysis->updatedAt);
    stmt.step();
}


std::vector<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalyses() const
{
    std::vector<std::shared_ptr<Analysis>> out;
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at FROM analyses ORDER BY id;");
    if (!stmt) {
        return out;
    }

    while (stmt.step() == SQLITE_ROW) {
        out.push_back(readAnalysis(stmt));
    }
    return out;
}

std::optional<std::shared_ptr<Analysis>> SqliteAnalysisRepository::getAnalysisById(const std::string& id) const
{
    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "SELECT id, name, type, config_json, filter_spec, export_format, include_calc_adjustments, export_state_json, snapshot_transactions_json, created_at, updated_at FROM analyses WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }
    return readAnalysis(stmt);
}

void SqliteAnalysisRepository::removeAnalysis(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM analyses WHERE id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, id);
    stmt.step();
}

void SqliteAnalysisRepository::updateAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id.empty()) {
        return;
    }

    persistence::StmtGuard stmt(pimpl_->db->handle(),
        "UPDATE analyses SET name=?, type=?, config_json=?, filter_spec=?, export_format=?, include_calc_adjustments=?, export_state_json=?, snapshot_transactions_json=?, created_at=?, updated_at=?"
        " WHERE id=?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, analysis->name);
    stmt.bindText(2, analysis->type);
    stmt.bindText(3, analysis->configJson);
    stmt.bindText(4, analysis->filterSpec);
    stmt.bindText(5, analysis->exportFormat);
    stmt.bindInt(6, analysis->includeCalcAdjustments ? 1 : 0);
    stmt.bindText(7, analysis->exportStateJson);
    stmt.bindText(8, analysis->snapshotTransactionsJson);
    stmt.bindText(9, analysis->createdAt);
    stmt.bindText(10, analysis->updatedAt);
    stmt.bindText(11, analysis->id);
    stmt.step();
}

void SqliteAnalysisRepository::upsertAnalysis(const std::shared_ptr<Analysis>& analysis)
{
    if (!analysis || analysis->id.empty()) {
        return;
    }

    updateAnalysis(analysis);
    if (sqlite3_changes(pimpl_->db->handle()) == 0) {
        addAnalysis(analysis);
    }
}

void SqliteAnalysisRepository::clearAnalyses()
{
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM analyses;", nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
}

sqlite3* SqliteAnalysisRepository::sqliteHandle() const noexcept {
    return pimpl_ && pimpl_->db ? pimpl_->db->handle() : nullptr;
}
