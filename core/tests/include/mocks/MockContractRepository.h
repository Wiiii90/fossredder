#pragma once

#include "core/repositories/IContractRepository.h"
#include <gmock/gmock.h>

class MockContractRepository : public IContractRepository {
public:
    MOCK_METHOD(void, addContract, (const std::shared_ptr<Contract>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Contract>>, getContracts, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Contract>>, getContractById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeContract, (const std::string&), (override));
    MOCK_METHOD(void, updateContract, (const std::shared_ptr<Contract>&), (override));
    MOCK_METHOD(void, upsertContract, (const std::shared_ptr<Contract>&), (override));
    MOCK_METHOD(void, clearContracts, (), (override));
};
