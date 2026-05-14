/**
 * @file persistence/tests/unit/TestWorkspaceStateStore.cpp
 * @brief Unit-style tests for `WorkspaceStateStore` using a temporary SQLite
 * database.
 */

#include "gtest/gtest.h"

#include "core/application/export/ExportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "persistence/Factory.h"
#include "persistence/WorkspaceStateStore.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::Statement;
using core::domain::Transaction;

namespace {

core::domain::Alias makeAlias(const std::string &value) {
  core::domain::Alias alias;
  alias.setValue(value);
  alias.setSource(value);
  return alias;
}

class TempDatabaseFile {
public:
  TempDatabaseFile() {
    const auto uniqueId = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path_ = std::filesystem::temp_directory_path() /
            ("fossredder-app-state-store-tests-" + uniqueId + ".db");
  }

  ~TempDatabaseFile() {
    std::error_code error;
    std::filesystem::remove(path_, error);
  }

  const std::filesystem::path &path() const noexcept { return path_; }

private:
  std::filesystem::path path_;
};

} // namespace

TEST(WorkspaceStateStoreTests,
     SaveReportsDeletionImpactForRemovedPersistedRows) {
  TempDatabaseFile tempDb;
  WorkspaceStateStore store(createSqliteDb(tempDb.path().string()));

  core::application::workspace::WorkspaceSessionState initialState;

  auto actor = std::make_shared<Actor>();
  actor->setId("actor-1");
  actor->rename("Alice");
  actor->setAliases({makeAlias("A. Example")});
  initialState.catalog.setActors({actor});

  auto property = std::make_shared<core::domain::Property>();
  property->setId("property-1");
  property->rename("Main Building");
  property->setAliases({makeAlias("HQ")});
  initialState.catalog.setProperties({property});

  auto contract = std::make_shared<core::domain::Contract>();
  contract->setId("contract-1");
  contract->rename("Rental Contract");
  contract->setType("rental");
  contract->setActorIds({"actor-1"});
  contract->setPropertyIds({"property-1"});
  contract->setAliases({makeAlias("C1")});
  initialState.catalog.setContracts({contract});

  auto statement = std::make_shared<Statement>();
  statement->setId("statement-1");
  statement->rename("January");
  initialState.catalog.setStatements({statement});

  auto transaction = std::make_shared<Transaction>();
  transaction->setId("transaction-1");
  transaction->setName("Rent");
  transaction->setStatementId(statement->id());
  initialState.catalog.setTransactions({transaction});

  store.save(initialState);

  const core::application::workspace::WorkspaceSessionState emptyState;
  const auto result = store.save(emptyState);

  EXPECT_EQ(result.impact.deletedActorIds,
            (std::vector<std::string>{"actor-1"}));
  EXPECT_EQ(result.impact.deletedStatementIds,
            (std::vector<std::string>{"statement-1"}));
  EXPECT_TRUE(result.impact.deletedTransactionIds.empty());

  const auto loaded = store.load();
  EXPECT_TRUE(loaded.empty());
}

TEST(WorkspaceStateStoreTests, PersistsAndLoadsCompleteAggregate) {
  TempDatabaseFile tempDb;
  WorkspaceStateStore store(createSqliteDb(tempDb.path().string()));

  core::application::workspace::WorkspaceSessionState state;

  auto actor = std::make_shared<Actor>();
  actor->setId("actor-1");
  actor->rename("Alice");
  actor->setAliases({makeAlias("A. Example")});
  state.catalog.setActors({actor});

  auto property = std::make_shared<core::domain::Property>();
  property->setId("property-1");
  property->rename("Main Building");
  property->setAliases({makeAlias("HQ")});
  state.catalog.setProperties({property});

  auto contract = std::make_shared<core::domain::Contract>();
  contract->setId("contract-1");
  contract->rename("Rental Contract");
  contract->setType("rental");
  contract->setActorIds({"actor-1"});
  contract->setPropertyIds({"property-1"});
  contract->setAliases({makeAlias("C1")});
  state.catalog.setContracts({contract});

  auto statement = std::make_shared<Statement>();
  statement->setId("statement-1");
  statement->rename("January Statement");
  state.catalog.setStatements({statement});

  auto transaction = std::make_shared<Transaction>();
  transaction->setId("tx-1");
  transaction->setName("Rent Payment");
  transaction->setBookingDate("2025-01-05");
  transaction->setValuta("2025-01-06");
  transaction->setAmount(450.25);
  transaction->setStatementId("statement-1");
  transaction->setContractId("contract-1");
  transaction->setActorId("actor-1");
  transaction->setStatus(Transaction::Status::Verified);
  transaction->setAllocatable(true);
  transaction->setPropertyIds({"property-1"});
  state.catalog.setTransactions({transaction});

  auto analysis = std::make_shared<Analysis>();
  analysis->setId("analysis-1");
  analysis->rename("Cashflow");
  analysis->setType("tab");
  analysis->setConfigJson(R"({"mode":"monthly"})");
  analysis->setFilterSpec("contract.type=rental");
  analysis->setCreatedAt("2025-01-01T00:00:00Z");
  analysis->setUpdatedAt("2025-01-02T00:00:00Z");
  state.catalog.setAnalyses({analysis});

  auto annual = std::make_shared<Annual>();
  annual->setId("annual-2025");
  annual->setYear(2025);
  annual->setAnalysisIds({"analysis-1"});
  annual->setCreatedAt("2025-01-01T00:00:00Z");
  annual->setUpdatedAt("2025-01-02T00:00:00Z");
  state.catalog.setAnnuals({annual});

  auto statementDraft =
      std::make_shared<core::application::importing::draft::StatementDraft>();
  statementDraft->name = "Imported January";
  state.workflow.statementDrafts.push_back(statementDraft);

  auto transactionDraft =
      std::make_shared<core::application::importing::draft::TransactionDraft>();
  transactionDraft->name = "Draft Rent";
  transactionDraft->bookingDate = "2025-01-04";
  transactionDraft->valuta = "2025-01-05";
  transactionDraft->amount = 449.99;
  transactionDraft->metadata = "ocr line";
  state.workflow.transactionDrafts.push_back(transactionDraft);

  auto exportLog = std::make_shared<core::application::exporting::ExportLog>();
  exportLog->id = "export-log-1";
  exportLog->time = "2025-01-03T00:00:00Z";
  exportLog->targetPath = "C:/exports/annual.zip";
  exportLog->status = "Success";
  exportLog->message = "Export completed";
  exportLog->payload = "{}";
  state.workflow.exportLogs.push_back(exportLog);

  store.save(state);
  const auto loaded = store.load();

  ASSERT_EQ(loaded.catalog.actors().size(), 1u);
  ASSERT_EQ(loaded.catalog.properties().size(), 1u);
  ASSERT_EQ(loaded.catalog.contracts().size(), 1u);
  ASSERT_EQ(loaded.catalog.statements().size(), 1u);
  ASSERT_EQ(loaded.catalog.transactions().size(), 1u);
  ASSERT_EQ(loaded.catalog.analyses().size(), 1u);
  ASSERT_EQ(loaded.catalog.annuals().size(), 1u);
  ASSERT_EQ(loaded.workflow.statementDrafts.size(), 1u);
  ASSERT_EQ(loaded.workflow.transactionDrafts.size(), 1u);
  ASSERT_EQ(loaded.workflow.exportLogs.size(), 1u);

  EXPECT_EQ(loaded.catalog.actors().front()->name(), "Alice");
  EXPECT_EQ(loaded.catalog.properties().front()->name(), "Main Building");
  EXPECT_EQ(loaded.catalog.contracts().front()->actorIds(),
            (std::vector<std::string>{"actor-1"}));
  EXPECT_EQ(loaded.catalog.contracts().front()->propertyIds(),
            (std::vector<std::string>{"property-1"}));
  EXPECT_EQ(loaded.catalog.statements().front()->name(), "January Statement");
  EXPECT_EQ(loaded.catalog.transactions().front()->statementId(), "statement-1");
  EXPECT_EQ(loaded.catalog.transactions().front()->contractId(), "contract-1");
  EXPECT_EQ(loaded.catalog.transactions().front()->status(),
            Transaction::Status::Verified);
  EXPECT_EQ(loaded.catalog.analyses().front()->filterSpec(),
            "contract.type=rental");
  EXPECT_EQ(loaded.catalog.annuals().front()->analysisIds(),
            (std::vector<std::string>{"analysis-1"}));
  EXPECT_EQ(loaded.workflow.statementDrafts.front()->name, "Imported January");
  EXPECT_EQ(loaded.workflow.transactionDrafts.front()->name, "Draft Rent");
  EXPECT_EQ(loaded.workflow.transactionDrafts.front()->metadata, "ocr line");
  EXPECT_EQ(loaded.workflow.exportLogs.front()->targetPath,
            "C:/exports/annual.zip");
  EXPECT_EQ(loaded.workflow.exportLogs.front()->status, "Success");
}
