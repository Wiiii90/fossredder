#pragma once

#include "core/repositories/IStatementRepository.h"
#include <gmock/gmock.h>

using core::domain::Statement;

class MockStatementRepository : public IStatementRepository {
public:
    MOCK_METHOD(void, addStatement, (const std::shared_ptr<Statement>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Statement>>, getStatements, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Statement>>, getStatementById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeStatement, (const std::string&), (override));
    MOCK_METHOD(void, updateStatement, (const std::shared_ptr<Statement>&), (override));
    MOCK_METHOD(void, upsertStatement, (const std::shared_ptr<Statement>&), (override));
    MOCK_METHOD(void, clearStatements, (), (override));
};
