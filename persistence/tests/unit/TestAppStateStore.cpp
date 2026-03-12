/**
 * @file persistence/tests/unit/TestAppStateStore.cpp
 * @brief Unit-style tests for `AppStateStore` using a temporary SQLite database.
 */

#include "gtest/gtest.h"

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/AppState.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "persistence/AppStateStore.h"
#include "persistence/Factory.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::AppState;
using core::domain::Statement;
using core::domain::Transaction;

namespace {

class TempDatabaseFile {
public:
    TempDatabaseFile()
    {
        const auto uniqueId = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        path_ = std::filesystem::temp_directory_path() /
                ("fossredder-app-state-store-tests-" + uniqueId + ".db");
    }

    ~TempDatabaseFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace

TEST(AppStateStoreTests, SaveAndLoadRoundTripsAnalysesAndAnnuals)
{
    TempDatabaseFile tempDb;
    AppStateStore store(createSqliteDb(tempDb.path().string()));

    AppState state;

    auto analysis = std::make_shared<Analysis>();
    analysis->id = "analysis-1";
    analysis->name = "Overview";
    analysis->type = "tab";
    analysis->configJson = R"({"scope":"year"})";
    analysis->filterSpec = "contract.type=rent";
    analysis->createdAt = "2025-01-01T00:00:00Z";
    analysis->updatedAt = "2025-01-02T00:00:00Z";
    state.analyses.push_back(analysis);

    auto annual = std::make_shared<Annual>();
    annual->id = "annual-2025";
    annual->year = 2025;
    annual->transactionIds = {"tx-1", "tx-2"};
    annual->assignedAnalysisIds = {analysis->id};
    annual->verificationState = Annual::VerificationState::Verified;
    annual->createdAt = "2025-01-01T00:00:00Z";
    annual->updatedAt = "2025-01-02T00:00:00Z";
    state.annuals.push_back(annual);

    store.save(state);
    const auto loaded = store.load();

    ASSERT_EQ(loaded.analyses.size(), 1u);
    ASSERT_EQ(loaded.annuals.size(), 1u);
    EXPECT_EQ(loaded.analyses.front()->id, "analysis-1");
    EXPECT_EQ(loaded.annuals.front()->id, "annual-2025");
    EXPECT_EQ(loaded.annuals.front()->assignedAnalysisIds, (std::vector<std::string>{"analysis-1"}));
}

TEST(AppStateStoreTests, SaveReportsDeletionImpactForRemovedPersistedRows)
{
    TempDatabaseFile tempDb;
    AppStateStore store(createSqliteDb(tempDb.path().string()));

    AppState initialState;

    auto actor = std::make_shared<Actor>();
    actor->id = "actor-1";
    actor->name = "Alice";
    initialState.actors.push_back(actor);

    auto statement = std::make_shared<Statement>();
    statement->id = "statement-1";
    statement->name = "January";
    initialState.statements.push_back(statement);

    auto transaction = std::make_shared<Transaction>();
    transaction->id = "transaction-1";
    transaction->name = "Rent";
    transaction->statementId = statement->id;
    initialState.transactions.push_back(transaction);

    store.save(initialState);

    const AppState emptyState;
    const auto result = store.save(emptyState);

    EXPECT_EQ(result.impact.deletedActorIds, (std::vector<std::string>{"actor-1"}));
    EXPECT_EQ(result.impact.deletedStatementIds, (std::vector<std::string>{"statement-1"}));
    EXPECT_TRUE(result.impact.deletedTransactionIds.empty());

    const auto loaded = store.load();
    EXPECT_TRUE(loaded.empty());
}
