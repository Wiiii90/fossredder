#pragma once

#include "core/repositories/IStatementDraftRepository.h"

#include <gmock/gmock.h>

using core::domain::StatementDraft;

class MockStatementDraftRepository : public IStatementDraftRepository {
public:
    MOCK_METHOD(void, addStatementDraft, (const std::shared_ptr<StatementDraft>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<StatementDraft>>, getStatementDrafts, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<StatementDraft>>, getStatementDraftById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeStatementDraft, (const std::string&), (override));
    MOCK_METHOD(void, updateStatementDraft, (const std::shared_ptr<StatementDraft>&), (override));
    MOCK_METHOD(void, upsertStatementDraft, (const std::shared_ptr<StatementDraft>&), (override));
    MOCK_METHOD(void, clearStatementDrafts, (), (override));
};
