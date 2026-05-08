/**
 * @file core/include/core/repositories/ITransactionDraftRepository.h
 * @brief Repository interface for persisting the current import transaction drafts.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class TransactionDraft;
}

class ITransactionDraftRepository {
public:
    virtual ~ITransactionDraftRepository() = default;

    /**
     * @brief Persist a transaction draft.
     * @param draft Shared pointer to the transaction draft to be persisted.
     */
    virtual void addTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;

    /**
     * @brief Retrieve all stored transaction drafts.
     * @return All transaction drafts stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDrafts() const = 0;

    /**
     * @brief Retrieve a transaction draft by its identifier.
     * @param id Core transaction draft identifier.
     * @return The transaction draft with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDraftById(const std::string& id) const = 0;

    /**
     * @brief Remove a transaction draft by its identifier.
     * @param id Core transaction draft identifier.
     */
    virtual void removeTransactionDraft(const std::string& id) = 0;

    /**
     * @brief Update an existing transaction draft.
     * @param draft Shared pointer to the transaction draft with updated values.
     */
    virtual void updateTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;

    /**
     * @brief Insert or update a transaction draft.
     * @param draft Shared pointer to the transaction draft to be upserted.
     */
    virtual void upsertTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;

    /**
     * @brief Remove all transaction drafts from the repository.
     */
    virtual void clearTransactionDrafts() = 0;

    /**
     * @brief Replace the stored transaction drafts with the provided drafts.
     * @param drafts Vector of shared pointers to the transaction drafts to be saved.
     */
    void saveTransactionDrafts(const std::vector<std::shared_ptr<core::domain::TransactionDraft>>& drafts);
};
