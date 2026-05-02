#pragma once

#include "core/repositories/IExportLogRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteExportLogRepository : public IExportLogRepository {
public:
    explicit SqliteExportLogRepository(const std::string& dbPath);
    explicit SqliteExportLogRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteExportLogRepository() override;

    void addExportLog(const std::shared_ptr<core::domain::ExportLog>& log) override;
    std::vector<std::shared_ptr<core::domain::ExportLog>> getExportLogs() const override;
    std::optional<std::shared_ptr<core::domain::ExportLog>> getExportLogById(const std::string& id) const override;
    void removeExportLog(const std::string& id) override;
    void updateExportLog(const std::shared_ptr<core::domain::ExportLog>& log) override;
    void upsertExportLog(const std::shared_ptr<core::domain::ExportLog>& log) override;
    void clearExportLogs() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
