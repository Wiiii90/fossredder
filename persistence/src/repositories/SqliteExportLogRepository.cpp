#include "persistence/repositories/SqliteExportLogRepository.h"

#include "core/models/ExportLog.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

void writeExportLog(sqlite3* db,
                    const std::shared_ptr<core::domain::ExportLog>& log,
                    const char* sql)
{
    if (!db || !log) {
        return;
    }

    if (log->id.empty()) {
        log->id = persistence::generateUuid();
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return;
    }

    stmt.bindText(1, log->id);
    stmt.bindText(2, log->time);
    stmt.bindText(3, log->targetPath);
    stmt.bindText(4, log->status);
    stmt.bindText(5, log->message);
    stmt.bindText(6, log->payload);
    stmt.step();
}

} // namespace

struct SqliteExportLogRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteExportLogRepository::SqliteExportLogRepository(const std::string& dbPath)
    : SqliteExportLogRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteExportLogRepository::SqliteExportLogRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteExportLogRepository::~SqliteExportLogRepository() = default;

void SqliteExportLogRepository::addExportLog(const std::shared_ptr<core::domain::ExportLog>& log)
{
    writeExportLog(
        pimpl_->db->handle(),
        log,
        "INSERT INTO export_logs (id, time, target_path, status, message, payload) VALUES (?, ?, ?, ?, ?, ?);");
}

std::vector<std::shared_ptr<core::domain::ExportLog>> SqliteExportLogRepository::getExportLogs() const
{
    std::vector<std::shared_ptr<core::domain::ExportLog>> logs;
    persistence::StmtGuard stmt(
        pimpl_->db->handle(),
        "SELECT id, time, target_path, status, message, payload FROM export_logs ORDER BY time DESC, id DESC;");
    if (!stmt) {
        return logs;
    }

    while (stmt.step() == SQLITE_ROW) {
        auto log = std::make_shared<core::domain::ExportLog>();
        log->id = stmt.columnText(0);
        log->time = stmt.columnText(1);
        log->targetPath = stmt.columnText(2);
        log->status = stmt.columnText(3);
        log->message = stmt.columnText(4);
        log->payload = stmt.columnText(5);
        logs.push_back(std::move(log));
    }

    return logs;
}

std::optional<std::shared_ptr<core::domain::ExportLog>> SqliteExportLogRepository::getExportLogById(const std::string& id) const
{
    persistence::StmtGuard stmt(
        pimpl_->db->handle(),
        "SELECT id, time, target_path, status, message, payload FROM export_logs WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    auto log = std::make_shared<core::domain::ExportLog>();
    log->id = stmt.columnText(0);
    log->time = stmt.columnText(1);
    log->targetPath = stmt.columnText(2);
    log->status = stmt.columnText(3);
    log->message = stmt.columnText(4);
    log->payload = stmt.columnText(5);
    return log;
}

void SqliteExportLogRepository::removeExportLog(const std::string& id)
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM export_logs WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteExportLogRepository::updateExportLog(const std::shared_ptr<core::domain::ExportLog>& log)
{
    writeExportLog(
        pimpl_->db->handle(),
        log,
        "UPDATE export_logs SET time = ?2, target_path = ?3, status = ?4, message = ?5, payload = ?6 WHERE id = ?1;");
}

void SqliteExportLogRepository::upsertExportLog(const std::shared_ptr<core::domain::ExportLog>& log)
{
    writeExportLog(
        pimpl_->db->handle(),
        log,
        "INSERT OR REPLACE INTO export_logs (id, time, target_path, status, message, payload) VALUES (?, ?, ?, ?, ?, ?);");
}

void SqliteExportLogRepository::clearExportLogs()
{
    persistence::StmtGuard stmt(pimpl_->db->handle(), "DELETE FROM export_logs;");
    if (!stmt) {
        return;
    }
    stmt.step();
}
