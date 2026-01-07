#pragma once

#include "core/repositories/ITransactionRepository.h"
#include <gmock/gmock.h>

class MockTransactionRepository : public ITransactionRepository {
public:
    MOCK_METHOD(void, addTransaction, (const std::shared_ptr<Transaction>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Transaction>>, getTransactions, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Transaction>>, getTransactionById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeTransaction, (const std::string&), (override));
    MOCK_METHOD(void, updateTransaction, (const std::shared_ptr<Transaction>&), (override));
    MOCK_METHOD(void, upsertTransaction, (const std::shared_ptr<Transaction>&), (override));
    MOCK_METHOD(void, clearTransactions, (), (override));
};
