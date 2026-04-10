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

    virtual void addStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;
    virtual std::vector<std::shared_ptr<core::domain::StatementDraft>> getStatementDrafts() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::StatementDraft>> getStatementDraftById(const std::string& id) const = 0;
    virtual void removeStatementDraft(const std::string& id) = 0;
    virtual void updateStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;
    virtual void upsertStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft) = 0;
    virtual void clearStatementDrafts() = 0;

    void saveStatementDraft(const std::shared_ptr<core::domain::StatementDraft>& draft)
    {
        clearStatementDrafts();
        if (draft) {
            upsertStatementDraft(draft);
        }
    }

    std::optional<std::shared_ptr<core::domain::StatementDraft>> getStatementDraft() const
    {
        const auto drafts = getStatementDrafts();
        if (drafts.empty()) {
            return std::nullopt;
        }
        return drafts.front();
    }

    void clearStatementDraft()
    {
        clearStatementDrafts();
    }
};
