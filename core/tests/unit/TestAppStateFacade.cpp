/**
 * @file core/tests/unit/TestAppStateFacade.cpp
 * @brief Unit tests for `AppStateFacade`.
 */

#include "gtest/gtest.h"

#include "core/application/AppStateFacade.h"
#include "core/constants/CoreDefaults.h"
#include "core/models/Actor.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/StatementDraft.h"
#include "core/models/Transaction.h"
#include "core/storage/IStorageManager.h"

#include <atomic>
#include <optional>
#include <string>
#include <utility>

using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Statement;
using core::domain::Transaction;

namespace {

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

    AppState loadFrom(const std::string& filePath) override {
        currentPath_ = filePath;
        AppState state;
        if (filePath == "db_with_actor") {
            auto actor = std::make_shared<Actor>();
            actor->id = "1";
            actor->name = "Alice";
            state.actors.push_back(actor);
        }
        return state;
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

} // namespace

TEST(AppStateFacadeTests, OpenLatestLoadsStateAndNotifies) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storage = fake.get();
    storage->nextLatestPath = "db_with_actor";

    core::application::AppStateFacade facade(std::move(fake));

    std::atomic<bool> notified{false};
    facade.setStateChangedCallback([&](const AppState& state) {
        notified = true;
        EXPECT_GT(state.actors.size(), 0u);
    });

    facade.openLatest();

    EXPECT_TRUE(notified);
    EXPECT_EQ(facade.state().actors.size(), 1u);
    EXPECT_EQ(facade.currentPath(), std::string("db_with_actor"));
}

TEST(AppStateFacadeTests, NewFileCreatesEmptyStateAndNotifies) {
    auto fake = std::make_unique<FakeStorageManager>();

    core::application::AppStateFacade facade(std::move(fake));

    std::atomic<bool> notified{false};
    facade.setStateChangedCallback([&](const AppState&) {
        notified = true;
    });

    facade.newFile("newdb");
    EXPECT_TRUE(notified);
    EXPECT_TRUE(facade.state().actors.empty());
    EXPECT_EQ(facade.currentPath(), std::string("newdb"));
}

TEST(AppStateFacadeTests, SaveFileDelegatesToStorage) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storage = fake.get();

    core::application::AppStateFacade facade(std::move(fake));
    facade.newFile("save-test.db");
    facade.addActor("Bob", {});
    storage->saveCalled = false;

    EXPECT_FALSE(storage->saveCalled);
    facade.saveFile();
    EXPECT_TRUE(storage->saveCalled);
    EXPECT_EQ(storage->lastSavedState.actors.size(), facade.state().actors.size());
    if (!storage->lastSavedState.actors.empty()) {
        EXPECT_EQ(storage->lastSavedState.actors.front()->name, std::string("Bob"));
    }
}

TEST(AppStateFacadeTests, NotifyStateNotifiesWithoutSaving) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storage = fake.get();

    core::application::AppStateFacade facade(std::move(fake));
    facade.newFile("notify-test.db");
    facade.addActor("Nina", {});
    storage->saveCalled = false;

    std::atomic<bool> notified{false};
    facade.setStateChangedCallback([&](const AppState& state) {
        notified = true;
        ASSERT_EQ(state.actors.size(), 1u);
        EXPECT_EQ(state.actors.front()->name, "Nina");
    });

    facade.notifyState();

    EXPECT_TRUE(notified);
    EXPECT_FALSE(storage->saveCalled);
}

TEST(AppStateFacadeTests, FinalizeStatementDraftMaterializesEntitiesAndCommits) {
    auto fake = std::make_unique<FakeStorageManager>();
    FakeStorageManager* storage = fake.get();

    core::application::AppStateFacade facade(std::move(fake));
    facade.newFile("draft.db");

    core::domain::StatementDraft draft;
    draft.name = "   ";
    core::domain::TransactionDraft draftTx;
    draftTx.name = "Rent";
    draftTx.bookingDate = "2025-01-01";
    draftTx.amount = 42.5;
    draftTx.description = "January";
    draftTx.actorId = "actor-1";
    draftTx.status = Transaction::Status::Verified;
    draftTx.type = " Utility ";
    draftTx.allocatable = true;
    draftTx.propertyIds = {"property-1"};
    draft.transactions.push_back(draftTx);

    const std::string statementId = facade.finalizeStatementDraft(draft);

    ASSERT_FALSE(statementId.empty());
    EXPECT_TRUE(storage->saveCalled);
    ASSERT_EQ(facade.state().statements.size(), 1u);
    EXPECT_EQ(facade.state().statements.front()->id, statementId);
    EXPECT_EQ(facade.state().statements.front()->name, core::constants::appState::kDefaultImportedStatementName);

    ASSERT_EQ(facade.state().transactions.size(), 1u);
    const auto& transaction = facade.state().transactions.front();
    EXPECT_EQ(transaction->statementId, statementId);
    EXPECT_EQ(transaction->status, Transaction::Status::Verified);
    EXPECT_TRUE(transaction->valuta.empty());

    ASSERT_EQ(facade.state().contracts.size(), 1u);
    const auto& generatedContract = facade.state().contracts.front();
    EXPECT_EQ(generatedContract->name, std::string(core::constants::appState::kGeneratedContractPrefix) + "1");
    EXPECT_EQ(generatedContract->type, "Utility");
}
