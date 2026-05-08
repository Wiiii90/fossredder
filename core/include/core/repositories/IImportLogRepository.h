/**
 * @file core/include/core/repositories/IImportLogRepository.h
 * @brief Repository interface for core::domain::ImportLog persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class ImportLog;
}

class IImportLogRepository {
public:
    virtual ~IImportLogRepository() = default;

    /**
     * @brief Insert a new import log into the repository.
     * @param log Shared pointer to the core::domain::ImportLog to add.
     */
    virtual void addImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;

    /**
     * @brief Retrieve all stored import logs.
     * @return All import logs stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::ImportLog>> getImportLogs() const = 0;

    /**
     * @brief Retrieve an import log by its identifier.
     * @param id Core import log identifier.
     * @return The import log with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::ImportLog>> getImportLogById(const std::string& id) const = 0;

    /**
     * @brief Remove an import log identified by id from the repository.
     * @param id core::domain::ImportLog identifier to remove.
     */
    virtual void removeImportLog(const std::string& id) = 0;

    /**
     * @brief Update an existing import log record in the repository.
     * @param log Shared pointer to the core::domain::ImportLog with updated fields.
     */
    virtual void updateImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;

    /**
     * @brief Upsert an import log: insert or update depending on existence.
     * @param log Shared pointer to the core::domain::ImportLog to upsert.
     */
    virtual void upsertImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;

    /**
     * @brief Remove all import logs from the repository.
     */
    virtual void clearImportLogs() = 0;
};
