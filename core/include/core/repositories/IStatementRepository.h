#pragma once

/**
 * @file core/include/core/repositories/IStatementRepository.h
 * @brief Repository interface for core::domain::Statement persistence operations.
 */

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class core::domain::Statement;
}

class IStatementRepository {
public:
    virtual ~IStatementRepository() = default;

    /**
     * @brief Insert a new statement into the repository.
     * @param statement Shared pointer to the core::domain::Statement to add.
     */
    virtual void addStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Return all statements stored in the repository.
     * @return Vector of shared_ptr to core::domain::Statement.
     */
    virtual std::vector<std::shared_ptr<core::domain::Statement>> getStatements() const = 0;

    /**
     * @brief Retrieve a statement by its identifier.
     * @param id core::domain::Statement identifier string.
     * @return Optional shared_ptr to core::domain::Statement if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Statement>> getStatementById(const std::string& id) const = 0;

    /**
     * @brief Remove a statement identified by id from the repository.
     * @param id core::domain::Statement identifier to remove.
     */
    virtual void removeStatement(const std::string& id) = 0;

    /**
     * @brief Update an existing statement record in the repository.
     * @param statement Shared pointer to the core::domain::Statement with updated fields.
     */
    virtual void updateStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Upsert a statement: insert or update depending on existence.
     * @param statement Shared pointer to the core::domain::Statement to upsert.
     */
    virtual void upsertStatement(const std::shared_ptr<core::domain::Statement>& statement) = 0;

    /**
     * @brief Remove all statements from the repository.
     */
    virtual void clearStatements() = 0;
};
