/**
 * @file core/include/core/repositories/IStatementRepository.h
 * @brief Repository interface for core::domain::Statement persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Statement;
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
     * @brief Retrieve all stored statements.
     * @return A vector of shared pointers to all statements stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::Statement>> getStatements() const = 0;

    /**
     * @brief Retrieve a statement by its identifier.
     * @param id Core statement identifier.
     * @return A shared pointer to the statement with the requested identifier, if found.
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
