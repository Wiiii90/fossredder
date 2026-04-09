#pragma once

#include "core/repositories/IAnnualRepository.h"
#include <gmock/gmock.h>

using core::domain::Annual;

class MockAnnualRepository : public IAnnualRepository {
public:
    MOCK_METHOD(void, addAnnual, (const std::shared_ptr<Annual>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Annual>>, getAnnuals, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Annual>>, getAnnualById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeAnnual, (const std::string&), (override));
    MOCK_METHOD(void, updateAnnual, (const std::shared_ptr<Annual>&), (override));
    MOCK_METHOD(void, upsertAnnual, (const std::shared_ptr<Annual>&), (override));
    MOCK_METHOD(void, clearAnnuals, (), (override));
};
