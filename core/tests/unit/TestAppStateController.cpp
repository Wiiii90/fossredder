/**
 * @file core/tests/TestAppStateController.cpp
 * @brief Unit tests for AppStateController (UNIT).
 */

#include "gtest/gtest.h"

#include "core/controllers/AppStateController.h"
#include "core/storage/IStorageManager.h"
#include "core/models/AppState.h"
#include "core/models/Actor.h"

#include <optional>
#include <string>
#include <utility>
#include <atomic>

// FakeStorageManager: simple test double implementing IStorageManager
class FakeStorageManager : public IStorageManager {
public:
    std::optional<std::string> nextLatestPath;
    bool saveCalled = false;
    AppState lastSavedState;
    std::string currentPath_;

    void setRepoFactory(RepoFactory) override {}
    void setAtomicStoreSave(AtomicStoreSave) override {}
    void setAtomicStoreLoad(AtomicStoreLoad) override {}
    void setDeletionImpactCallback(DeletionImpactCallback) override {}

    std::optional<std::string> loadLatestPath() const override {
        return nextLatestPath;
    }

    void setLatestPath(const std::string& filePath) override {
        currentPath_ = filePath;
    }

    AppState load() override {
        return loadFrom(currentPath_);
    }

    AppState loadFrom(const std::string& filePath) override {
        currentPath_ = filePath;
        AppState s;
        if (filePath == "db_with_actor") {
            auto a = std::make_shared<Actor>();
            a->id = "1";
            a->name = "Alice";
            s.actors.push_back(a);
        }
        return s;
    }

    void save(const AppState& state) override {
        saveCalled = true;
        lastSavedState = state;
    }

    void saveAs(const std::string& filePath, const AppState& state) override {
        currentPath_ = filePath;
        save(state);
    }

    void createNew(const std::string& filePath) override {
        currentPath_ = filePath;
    }

    const std::string& currentPath() const noexcept override {
        return currentPath_;
    }
};

TEST(AppStateControllerTests, OpenLatest_loads_state_and_notifies) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* fptr = fake.get();
    fptr->nextLatestPath = "db_with_actor";

    core::controllers::AppStateController ctrl(std::move(fake));

    std::atomic<bool> notified{false};
    ctrl.setStateChangedCallback([&](const AppState& s){
        notified = true;
        EXPECT_GT(s.actors.size(), 0u);
    });

    ctrl.openLatest();

    EXPECT_TRUE(notified);
    EXPECT_EQ(ctrl.state().actors.size(), 1u);
    EXPECT_EQ(ctrl.currentPath(), std::string("db_with_actor"));
}

TEST(AppStateControllerTests, NewFile_creates_empty_state_and_notifies) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* fptr = fake.get();

    core::controllers::AppStateController ctrl(std::move(fake));

    std::atomic<bool> notified{false};
    ctrl.setStateChangedCallback([&](const AppState& s){
        notified = true;
    });

    ctrl.newFile("newdb");
    EXPECT_TRUE(notified);
    EXPECT_TRUE(ctrl.state().actors.empty());
    EXPECT_EQ(ctrl.currentPath(), std::string("newdb"));
}

TEST(AppStateControllerTests, SaveFile_delegates_to_storage) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* fptr = fake.get();

    core::controllers::AppStateController ctrl(std::move(fake));

    auto a = std::make_shared<Actor>();
    a->id = "x";
    a->name = "Bob";
    ctrl.mutableState().actors.push_back(a);

    EXPECT_FALSE(fptr->saveCalled);
    ctrl.saveFile();
    EXPECT_TRUE(fptr->saveCalled);
    EXPECT_EQ(fptr->lastSavedState.actors.size(), ctrl.state().actors.size());
    if (!fptr->lastSavedState.actors.empty()) {
        EXPECT_EQ(fptr->lastSavedState.actors.front()->name, std::string("Bob"));
    }
}
