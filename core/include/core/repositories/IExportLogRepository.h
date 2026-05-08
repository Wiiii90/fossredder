/**
 * @file core/include/core/repositories/IExportLogRepository.h
 * @brief Repository interface for core::domain::ExportLog persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class ExportLog;
}

class IExportLogRepository {
public:
    virtual ~IExportLogRepository() = default;

    /**
     * @brief Insert a new export log into the repository.
     * @param log Shared pointer to the core::domain::ExportLog to add.
     */
    virtual void addExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;

    /**
     * @brief Retrieve all stored export logs.
     * @return All export logs stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::ExportLog>> getExportLogs() const = 0;

    /**
     * @brief Retrieve an export log by its identifier.
     * @param id Core export log identifier.
     * @return The export log with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::ExportLog>> getExportLogById(const std::string& id) const = 0;

    /**
     * @brief Remove an export log identified by id from the repository.
     * @param id core::domain::ExportLog identifier to remove.
     */
    virtual void removeExportLog(const std::string& id) = 0;

    /**
     * @brief Update an existing export log record in the repository.
     * @param log Shared pointer to the core::domain::ExportLog with updated fields.
     */
    virtual void updateExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;

    /**
     * @brief Upsert an export log: insert or update depending on existence.
     * @param log Shared pointer to the core::domain::ExportLog to upsert.
     */
    virtual void upsertExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;

    /**
     * @brief Remove all export logs from the repository.
     */
    virtual void clearExportLogs() = 0;
};
