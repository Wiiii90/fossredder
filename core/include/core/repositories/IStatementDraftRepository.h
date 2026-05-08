/**
 * @file core/include/core/repositories/IStatementDraftRepository.h
 * @brief Repository interface for persisting the current import statement draft.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class StatementDraft;
}

class IStatementDraftRepository {
public:
    virtual ~IStatementDraftRepository() = default;

    /**
     * @brief Insert a new statement draft into the repository.
     * @param draft Shared pointer to the core::domain::StatementDraft to add.
     */
    virtual void addStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;

    /**
     * @brief Retrieve all stored statement drafts.
     * @return All statement drafts stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::StatementDraft>> getStatementDrafts() const = 0;

    /**
     * @brief Retrieve a statement draft by its identifier.
     * @param id Core statement draft identifier.
     * @return The statement draft with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::StatementDraft>> getStatementDraftById(const std::string& id) const = 0;

    /**
     * @brief Remove a statement draft identified by id from the repository.
     * @param id Core statement draft identifier.
     */
    virtual void removeStatementDraft(const std::string& id) = 0;

    /**
     * @brief Update an existing statement draft record in the repository.
     * @param draft Shared pointer to the core::domain::StatementDraft with updated fields.
     */
    virtual void updateStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;

    /**
     * @brief Insert or update a statement draft.
     * @param draft Shared pointer to the core::domain::StatementDraft to upsert.
     */
    virtual void upsertStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;

    /**
     * @brief Remove all statement drafts from the repository.
     */
    virtual void clearStatementDrafts() = 0;

    /**
     * @brief Replace the stored statement draft with the provided draft.
     * @param draft Shared pointer to the statement draft to save.
     */
    void saveStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft);

    /**
     * @brief Retrieve the first stored statement draft, if any.
     * @return The first stored statement draft, if one exists.
     */
    std::optional<std::shared_ptr<core::domain::StatementDraft>> getStatementDraft() const;

    /**
     * @brief Remove the stored statement draft.
     */
    void clearStatementDraft();
};
