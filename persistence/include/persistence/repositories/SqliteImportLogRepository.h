#pragma once

#include "core/repositories/IImportLogRepository.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteImportLogRepository : public IImportLogRepository {
public:
    explicit SqliteImportLogRepository(const std::string& dbPath);
    explicit SqliteImportLogRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteImportLogRepository() override;

    void addImportLog(const std::shared_ptr<core::domain::ImportLog>& log) override;
    std::vector<std::shared_ptr<core::domain::ImportLog>> getImportLogs() const override;
    std::optional<std::shared_ptr<core::domain::ImportLog>> getImportLogById(const std::string& id) const override;
    void removeImportLog(const std::string& id) override;
    void updateImportLog(const std::shared_ptr<core::domain::ImportLog>& log) override;
    void upsertImportLog(const std::shared_ptr<core::domain::ImportLog>& log) override;
    void clearImportLogs() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
