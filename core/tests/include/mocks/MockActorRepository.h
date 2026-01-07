#pragma once

#include "core/repositories/IActorRepository.h"
#include <gmock/gmock.h>

class MockActorRepository : public IActorRepository {
public:
    MOCK_METHOD(void, addActor, (const std::shared_ptr<Actor>&), (override));
    MOCK_METHOD(std::vector<std::shared_ptr<Actor>>, getActors, (), (const, override));
    MOCK_METHOD(std::optional<std::shared_ptr<Actor>>, getActorById, (const std::string&), (const, override));
    MOCK_METHOD(void, removeActor, (const std::string&), (override));
    MOCK_METHOD(void, updateActor, (const std::shared_ptr<Actor>&), (override));
    MOCK_METHOD(void, upsertActor, (const std::shared_ptr<Actor>&), (override));
    MOCK_METHOD(void, clearActors, (), (override));
};
