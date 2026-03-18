/**
 * @file core/tests/unit/TestWorkspaceSession.cpp
 * @brief Unit tests for `core::application::WorkspaceSession`.
 */

#include "gtest/gtest.h"

#include "core/application/WorkspaceSession.h"
#include "core/models/Actor.h"
#include "core/storage/IStorageManager.h"

#include <memory>
#include <optional>
#include <string>

using core::domain::Actor;
using core::domain::AppState;

namespace {

class FakeStorageManager final : public core::storage::IStorageManager {
public:
    void setRepoFactory(RepoFactory) override {}
    void setAtomicStoreSave(AtomicStoreSave) override {}
    void setAtomicStoreLoad(AtomicStoreLoad) override {}
    void setDeletionImpactCallback(DeletionImpactCallback cb) override { deletionImpactCallback = std::move(cb); }

    std::optional<std::string> loadLatestPath() const override { return latestPath; }
    AppState loadFrom(const std::string& filePath) override { currentPathValue = filePath; return loadedState; }
    void save(const AppState& state) override { ++saveCount; lastSavedState = state; }
    void saveAs(const std::string& filePath, const AppState& state) override { currentPathValue = filePath; ++saveAsCount; lastSavedState = state; }
    void createNew(const std::string& filePath) override { currentPathValue = filePath; ++createNewCount; }
    const std::string& currentPath() const noexcept override { return currentPathValue; }

    std::optional<std::string> latestPath;
    std::string currentPathValue;
    AppState loadedState;
    AppState lastSavedState;
    int saveCount = 0;
    int saveAsCount = 0;
    int createNewCount = 0;
    DeletionImpactCallback deletionImpactCallback;
};

} // namespace

TEST(WorkspaceSessionTests, CommitSavesStateWhenCurrentPathIsPresent)
{
    auto storage = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storagePtr = storage.get();
    storagePtr->currentPathValue = "session.db";

    core::application::WorkspaceSession session(std::move(storage));
    auto actor = std::make_shared<Actor>();
    actor->id = "actor-1";
    actor->name = "Alice";
    session.mutableState().actors.push_back(actor);

    bool notified = false;
    session.setStateChangedCallback([&](const AppState& state) {
        notified = true;
        ASSERT_EQ(state.actors.size(), 1u);
        EXPECT_EQ(state.actors.front()->name, "Alice");
    });

    session.commit();

    EXPECT_EQ(storagePtr->saveCount, 1);
    ASSERT_EQ(storagePtr->lastSavedState.actors.size(), 1u);
    EXPECT_TRUE(notified);
}

TEST(WorkspaceSessionTests, CommitSkipsSaveWhenCurrentPathIsEmptyButStillNotifies)
{
    auto storage = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storagePtr = storage.get();

    core::application::WorkspaceSession session(std::move(storage));
    bool notified = false;
    session.setStateChangedCallback([&](const AppState&) { notified = true; });

    session.commit();

    EXPECT_EQ(storagePtr->saveCount, 0);
    EXPECT_TRUE(notified);
}

TEST(WorkspaceSessionTests, ConstructorRejectsNullStorageManager)
{
    EXPECT_THROW(core::application::WorkspaceSession session(nullptr), std::invalid_argument);
}
