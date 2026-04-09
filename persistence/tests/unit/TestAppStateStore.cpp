/**
 * @file persistence/tests/unit/TestAppStateStore.cpp
 * @brief Unit-style tests for `AppStateStore` using a temporary SQLite database.
 */

#include "gtest/gtest.h"

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/StatementDraft.h"
#include "core/models/Transaction.h"
#include "core/models/TransactionDraft.h"
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

TEST(AppStateStoreTests, PersistsAndLoadsCompleteAggregate)
{
    TempDatabaseFile tempDb;
    AppStateStore store(createSqliteDb(tempDb.path().string()));

    AppState state;

    auto actor = std::make_shared<Actor>();
    actor->id = "actor-1";
    actor->name = "Alice";
    actor->type = "person";
    actor->description = "primary actor";
    actor->aliases = {"A. Example"};
    state.actors.push_back(actor);

    auto property = std::make_shared<core::domain::Property>();
    property->id = "property-1";
    property->name = "Main Building";
    property->address = "Main St 1";
    property->description = "central asset";
    property->aliases = {"HQ"};
    state.properties.push_back(property);

    auto contract = std::make_shared<core::domain::Contract>();
    contract->id = "contract-1";
    contract->name = "Rental Contract";
    contract->type = "rental";
    contract->description = "primary contract";
    contract->actorIds = {"actor-1"};
    contract->propertyIds = {"property-1"};
    contract->aliases = {"C1"};
    state.contracts.push_back(contract);

    auto statement = std::make_shared<Statement>();
    statement->id = "statement-1";
    statement->name = "January Statement";
    state.statements.push_back(statement);

    auto transaction = std::make_shared<Transaction>();
    transaction->id = "tx-1";
    transaction->name = "Rent Payment";
    transaction->bookingDate = "2025-01-05";
    transaction->valuta = "2025-01-06";
    transaction->amount = 450.25;
    transaction->description = "monthly payment";
    transaction->statementId = "statement-1";
    transaction->contractId = "contract-1";
    transaction->actorId = "actor-1";
    transaction->status = Transaction::Status::Verified;
    transaction->allocatable = true;
    transaction->propertyIds = {"property-1"};
    state.transactions.push_back(transaction);

    auto analysis = std::make_shared<Analysis>();
    analysis->id = "analysis-1";
    analysis->name = "Cashflow";
    analysis->type = "tab";
    analysis->configJson = R"({"mode":"monthly"})";
    analysis->filterSpec = "contract.type=rental";
    analysis->createdAt = "2025-01-01T00:00:00Z";
    analysis->updatedAt = "2025-01-02T00:00:00Z";
    state.analyses.push_back(analysis);

    auto annual = std::make_shared<Annual>();
    annual->id = "annual-2025";
    annual->year = 2025;
    annual->transactionIds = {"tx-1"};
    annual->assignedAnalysisIds = {"analysis-1"};
    annual->verificationState = Annual::VerificationState::Verified;
    annual->createdAt = "2025-01-01T00:00:00Z";
    annual->updatedAt = "2025-01-02T00:00:00Z";
    state.annuals.push_back(annual);

    auto statementDraft = std::make_shared<core::domain::StatementDraft>();
    statementDraft->name = "Imported January";
    state.statementDrafts.push_back(statementDraft);

    auto transactionDraft = std::make_shared<core::domain::TransactionDraft>();
    transactionDraft->name = "Draft Rent";
    transactionDraft->bookingDate = "2025-01-04";
    transactionDraft->valuta = "2025-01-05";
    transactionDraft->amount = 449.99;
    transactionDraft->description = "draft payment";
    transactionDraft->metadata = "ocr line";
    state.transactionDrafts.push_back(transactionDraft);

    store.save(state);
    const auto loaded = store.load();

    ASSERT_EQ(loaded.actors.size(), 1u);
    ASSERT_EQ(loaded.properties.size(), 1u);
    ASSERT_EQ(loaded.contracts.size(), 1u);
    ASSERT_EQ(loaded.statements.size(), 1u);
    ASSERT_EQ(loaded.transactions.size(), 1u);
    ASSERT_EQ(loaded.analyses.size(), 1u);
    ASSERT_EQ(loaded.annuals.size(), 1u);
    ASSERT_EQ(loaded.statementDrafts.size(), 1u);
    ASSERT_EQ(loaded.transactionDrafts.size(), 1u);

    EXPECT_EQ(loaded.actors.front()->name, "Alice");
    EXPECT_EQ(loaded.properties.front()->address, "Main St 1");
    EXPECT_EQ(loaded.contracts.front()->actorIds, (std::vector<std::string>{"actor-1"}));
    EXPECT_EQ(loaded.contracts.front()->propertyIds, (std::vector<std::string>{"property-1"}));
    EXPECT_EQ(loaded.statements.front()->name, "January Statement");
    EXPECT_EQ(loaded.transactions.front()->statementId, "statement-1");
    EXPECT_EQ(loaded.transactions.front()->contractId, "contract-1");
    EXPECT_EQ(loaded.transactions.front()->status, Transaction::Status::Verified);
    EXPECT_EQ(loaded.analyses.front()->filterSpec, "contract.type=rental");
    EXPECT_EQ(loaded.annuals.front()->assignedAnalysisIds, (std::vector<std::string>{"analysis-1"}));
    EXPECT_EQ(loaded.statementDrafts.front()->name, "Imported January");
    EXPECT_EQ(loaded.transactionDrafts.front()->name, "Draft Rent");
    EXPECT_EQ(loaded.transactionDrafts.front()->metadata, "ocr line");
}
