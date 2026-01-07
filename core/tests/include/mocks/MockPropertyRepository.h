#pragma once

#include "core/repositories/IPropertyRepository.h"
#include <gmock/gmock.h>

class MockPropertyRepository : public IPropertyRepository {
public:
    MOCK_METHOD(void, addProperty, (const std::shared_ptr<Property>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Property>>, getProperties, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Property>>, getPropertyById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeProperty, (const std::string&), (override));
    MOCK_METHOD(void, updateProperty, (const std::shared_ptr<Property>&), (override));
    MOCK_METHOD(void, upsertProperty, (const std::shared_ptr<Property>&), (override));
    MOCK_METHOD(void, clearProperties, (), (override));
};
