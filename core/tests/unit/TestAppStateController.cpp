/**
 * @file core/tests/TestAppStateController.cpp
 * @brief Unit tests for AppStateController (UNIT).
 */

#include "gtest/gtest.h"

#include "core/controllers/AppStateController.h"
#include "core/constants/CoreDefaults.h"
#include "core/storage/IStorageManager.h"
#include "core/models/AppState.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/DraftStatement.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include <optional>
#include <string>
#include <utility>
#include <atomic>

using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Statement;
using core::domain::Transaction;

// FakeStorageManager: simple test double implementing IStorageManager
class FakeStorageManager : public core::storage::IStorageManager {
public:
    std::optional<std::string> nextLatestPath;
    bool saveCalled = false;
    AppState lastSavedState;
    std::string currentPath_;

    void setRepoFactory(core::storage::IStorageManager::RepoFactory) override {}
    void setAtomicStoreSave(core::storage::IStorageManager::AtomicStoreSave) override {}
    void setAtomicStoreLoad(core::storage::IStorageManager::AtomicStoreLoad) override {}
    void setDeletionImpactCallback(core::storage::IStorageManager::DeletionImpactCallback) override {}

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
    ctrl.newFile("save-test.db");
    ctrl.addActor("Bob", "", "");
    fptr->saveCalled = false;

    EXPECT_FALSE(fptr->saveCalled);
    ctrl.saveFile();
    EXPECT_TRUE(fptr->saveCalled);
    EXPECT_EQ(fptr->lastSavedState.actors.size(), ctrl.state().actors.size());
    if (!fptr->lastSavedState.actors.empty()) {
        EXPECT_EQ(fptr->lastSavedState.actors.front()->name, std::string("Bob"));
    }
}

TEST(AppStateControllerTests, NotifyState_notifies_without_saving) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* fptr = fake.get();

    core::controllers::AppStateController ctrl(std::move(fake));
    ctrl.newFile("notify-test.db");
    ctrl.addActor("Nina", "", "");
    fptr->saveCalled = false;

    std::atomic<bool> notified{false};
    ctrl.setStateChangedCallback([&](const AppState& state) {
        notified = true;
        ASSERT_EQ(state.actors.size(), 1u);
        EXPECT_EQ(state.actors.front()->name, "Nina");
    });

    ctrl.notifyState();

    EXPECT_TRUE(notified);
    EXPECT_FALSE(fptr->saveCalled);
}

TEST(AppStateControllerTests, FinalizeStatementDraft_materializes_entities_and_commits) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* fptr = fake.get();

    core::controllers::AppStateController ctrl(std::move(fake));
    ctrl.newFile("draft.db");

    core::domain::DraftStatement draft;
    draft.name = "   ";
    draft.transactions.push_back(core::domain::DraftTransaction{
        .name = "Rent",
        .bookingDate = "2025-01-01",
        .amount = 42.5,
        .description = "January",
        .status = Transaction::Status::Verified,
        .actorId = "actor-1",
        .allocatable = true,
        .propertyIds = {"property-1"},
        .type = " Utility "
    });

    const std::string statementId = ctrl.finalizeStatementDraft(draft);

    ASSERT_FALSE(statementId.empty());
    EXPECT_TRUE(fptr->saveCalled);
    ASSERT_EQ(ctrl.state().statements.size(), 1u);
    EXPECT_EQ(ctrl.state().statements.front()->id, statementId);
    EXPECT_EQ(ctrl.state().statements.front()->name, core::constants::appState::kDefaultImportedStatementName);

    ASSERT_EQ(ctrl.state().transactions.size(), 1u);
    const auto& transaction = ctrl.state().transactions.front();
    EXPECT_EQ(transaction->statementId, statementId);
    EXPECT_EQ(transaction->status, Transaction::Status::Verified);
    EXPECT_TRUE(transaction->valuta.empty());

    ASSERT_EQ(ctrl.state().contracts.size(), 1u);
    const auto& generatedContract = ctrl.state().contracts.front();
    EXPECT_EQ(generatedContract->name, std::string(core::constants::appState::kGeneratedContractPrefix) + "1");
    EXPECT_EQ(generatedContract->type, "Utility");
    EXPECT_EQ(transaction->contractId, generatedContract->id);
}
