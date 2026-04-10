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

    virtual void addTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;
    virtual std::vector<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDrafts() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::TransactionDraft>> getTransactionDraftById(const std::string& id) const = 0;
    virtual void removeTransactionDraft(const std::string& id) = 0;
    virtual void updateTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;
    virtual void upsertTransactionDraft(const std::shared_ptr<core::domain::TransactionDraft>& draft) = 0;
    virtual void clearTransactionDrafts() = 0;

    void saveTransactionDrafts(const std::vector<std::shared_ptr<core::domain::TransactionDraft>>& drafts)
    {
        clearTransactionDrafts();
        for (const auto& draft : drafts) {
            if (!draft) continue;
            upsertTransactionDraft(draft);
        }
    }
};
