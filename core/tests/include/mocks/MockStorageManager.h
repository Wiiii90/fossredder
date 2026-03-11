#pragma once

#include "core/storage/IStorageManager.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <optional>
#include <string>

class MockStorageManager : public core::storage::IStorageManager {
public:
    MOCK_METHOD(void, setRepoFactory, (RepoFactory), (override));
    MOCK_METHOD(void, setAtomicStoreSave, (AtomicStoreSave), (override));
    MOCK_METHOD(void, setAtomicStoreLoad, (AtomicStoreLoad), (override));
    MOCK_METHOD(void, setDeletionImpactCallback, (DeletionImpactCallback), (override));

    MOCK_METHOD(std::optional<std::string>, loadLatestPath, (), (const, override));
    MOCK_METHOD(void, setLatestPath, (const std::string&), (override));

    MOCK_METHOD(AppState, load, (), (override));
    MOCK_METHOD(AppState, loadFrom, (const std::string&), (override));

    MOCK_METHOD(void, save, (const AppState&), (override));
    MOCK_METHOD(void, saveAs, (const std::string&, const AppState&), (override));
    MOCK_METHOD(void, createNew, (const std::string&), (override));

    MOCK_METHOD(const std::string&, currentPath, (), (const, noexcept, override));
};
