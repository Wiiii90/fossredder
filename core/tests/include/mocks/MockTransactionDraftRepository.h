#pragma once

#include "core/repositories/ITransactionDraftRepository.h"

#include <gmock/gmock.h>

using core::domain::TransactionDraft;

class MockTransactionDraftRepository : public ITransactionDraftRepository {
public:
    MOCK_METHOD(void, addTransactionDraft, (const std::shared_ptr<TransactionDraft>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<TransactionDraft>>, getTransactionDrafts, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<TransactionDraft>>, getTransactionDraftById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeTransactionDraft, (const std::string&), (override));
    MOCK_METHOD(void, updateTransactionDraft, (const std::shared_ptr<TransactionDraft>&), (override));
    MOCK_METHOD(void, upsertTransactionDraft, (const std::shared_ptr<TransactionDraft>&), (override));
    MOCK_METHOD(void, clearTransactionDrafts, (), (override));
};
