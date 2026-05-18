/**
 * @file ui/tests/support/WorkspaceTestData.h
 * @brief Shared test data builders for UI source tests.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Alias.h"

namespace ui::tests::support {

inline core::domain::Alias makeAlias(const std::string& value,
                                     const std::string& kind = {},
                                     const std::string& source = {})
{
    return core::domain::Alias(value, kind, source);
}

inline std::shared_ptr<core::domain::Actor> makeActor()
{
    auto actor = std::make_shared<core::domain::Actor>();
    actor->setId("actor-1");
    actor->rename("Main Actor");
    actor->addAlias(makeAlias("Primary Actor"));
    actor->addAlias(makeAlias("Main Actor"));
    actor->setContractIds({"contract-1"});
    actor->setCreatedAt("2026-01-01T08:00:00Z");
    actor->setUpdatedAt("2026-01-02T08:00:00Z");
    return actor;
}

inline std::shared_ptr<core::domain::Property> makeProperty()
{
    auto property = std::make_shared<core::domain::Property>();
    property->setId("property-1");
    property->rename("Primary Property");
    property->addAlias(makeAlias("Property Alias"));
    property->setContractIds({"contract-1"});
    property->setCreatedAt("2026-01-01T08:00:00Z");
    property->setUpdatedAt("2026-01-02T08:00:00Z");
    return property;
}

inline std::shared_ptr<core::domain::Contract> makeContract()
{
    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Lease Contract");
    contract->setType("lease");
    contract->setActorIds({"actor-1"});
    contract->setPropertyIds({"property-1"});
    contract->addAlias(makeAlias("Lease"));
    contract->setCreatedAt("2026-01-01T08:00:00Z");
    contract->setUpdatedAt("2026-01-02T08:00:00Z");
    return contract;
}

inline std::shared_ptr<core::domain::Statement> makeStatement()
{
    auto statement = std::make_shared<core::domain::Statement>();
    statement->setId("statement-1");
    statement->rename("January Statement");
    statement->setTransactionIds({"tx-1", "tx-2"});
    statement->setCreatedAt("2026-01-01T08:00:00Z");
    statement->setUpdatedAt("2026-01-02T08:00:00Z");
    return statement;
}

inline std::shared_ptr<core::domain::Transaction> makeTransaction(const std::string& id,
                                                                  const std::string& name,
                                                                  const std::string& bookingDate,
                                                                  double amount,
                                                                  const std::string& statementId,
                                                                  bool allocatable,
                                                                  const std::string& contractId = "contract-1",
                                                                  const std::string& actorId = "actor-1",
                                                                  std::vector<std::string> propertyIds = {"property-1"},
                                                                  core::domain::Transaction::Status status = core::domain::Transaction::Status::Verified)
{
    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId(id);
    transaction->setName(name);
    transaction->setBookingDate(bookingDate);
    transaction->setValuta("EUR");
    transaction->setAmount(amount);
    transaction->setStatementId(statementId);
    transaction->setContractId(contractId);
    transaction->setActorId(actorId);
    transaction->setPropertyIds(std::move(propertyIds));
    transaction->setAllocatable(allocatable);
    transaction->setStatus(status);
    transaction->setCreatedAt("2026-01-01T08:00:00Z");
    transaction->setUpdatedAt("2026-01-02T08:00:00Z");
    return transaction;
}

inline std::shared_ptr<core::domain::Analysis> makeAnalysis()
{
    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Monthly Analysis");
    analysis->setType("tabular");
    analysis->setConfigJson("{\"groupBy\":\"month\"}");
    analysis->setFilterSpec("{}");
    analysis->setExportFormat("csv");
    analysis->setIncludeCalculationAdjustments(true);
    analysis->setExportStateJson("{}");
    analysis->setSnapshotTransactionsJson("[\"tx-1\"]");
    analysis->setAdjustment("actor-1", 19.25);
    analysis->setCreatedAt("2026-01-01T08:00:00Z");
    analysis->setUpdatedAt("2026-01-02T08:00:00Z");
    return analysis;
}

inline std::shared_ptr<core::domain::Annual> makeAnnual()
{
    auto annual = std::make_shared<core::domain::Annual>();
    annual->setId("annual-1");
    annual->rename("2026");
    annual->setYear(2026);
    annual->setAnalysisIds({"analysis-1"});
    annual->setCreatedAt("2026-01-01T08:00:00Z");
    annual->setUpdatedAt("2026-01-02T08:00:00Z");
    return annual;
}

inline core::domain::catalog::WorkspaceCatalog makeWorkspaceCatalog()
{
    core::domain::catalog::WorkspaceCatalog catalog;
    catalog.setActors({makeActor()});
    catalog.setProperties({makeProperty()});
    catalog.setContracts({makeContract()});
    catalog.setStatements({makeStatement()});
    catalog.setTransactions({
        makeTransaction("tx-1", "Rent", "2026-01-05", 1250.0, "statement-1", true),
        makeTransaction("tx-2", "Fees", "2026-01-06", -35.5, "statement-1", false, {}, {}, {"property-1"}, core::domain::Transaction::Status::Unverified)
    });
    catalog.setAnalyses({makeAnalysis()});
    catalog.setAnnuals({makeAnnual()});
    return catalog;
}

} // namespace ui::tests::support
