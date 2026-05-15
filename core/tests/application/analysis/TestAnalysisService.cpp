/**
 * @file core/tests/application/analysis/TestAnalysisService.cpp
 * @brief Tests for analysis application execution.
 */

#include <gtest/gtest.h>

#include "core/application/analysis/AnalysisService.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Transaction.h"

namespace core::application::analysis {

TEST(AnalysisServiceTest, RunsAnalysisAgainstWorkspaceState) {
    core::domain::catalog::WorkspaceCatalog state;

    auto contract = std::make_shared<core::domain::Contract>();
    contract->setId("contract-1");
    contract->rename("Contract");
    contract->setType("rent");
    state.setContracts({contract});

    auto transaction = std::make_shared<core::domain::Transaction>();
    transaction->setId("tx-1");
    transaction->setName("Rent");
    transaction->setBookingDate("2024-01-31");
    transaction->setAmount(10.0);
    transaction->setContractId("contract-1");
    state.setTransactions({transaction});

    auto analysis = std::make_shared<core::domain::Analysis>();
    analysis->setId("analysis-1");
    analysis->rename("Analysis");
    analysis->setType("tab");
    analysis->setExportFormat("xlsx");
    analysis->setConfigJson(R"({"group":"month"})");
    state.setAnalyses({analysis});

    AnalysisService service;
    const auto result = service.runAnalysisById(state, "analysis-1");

    EXPECT_TRUE(result.found);
    EXPECT_EQ(result.type, "tab");
    EXPECT_EQ(result.transactions.size(), 1u);
    EXPECT_EQ(result.transactions.front().contractType, "rent");
}

TEST(AnalysisServiceTest, ReturnsUnfoundResultWhenAnalysisIdIsMissing) {
    core::domain::catalog::WorkspaceCatalog state;
    AnalysisService service;

    const auto result = service.runAnalysisById(state, "missing");

    EXPECT_FALSE(result.found);
    EXPECT_TRUE(result.transactions.empty());
}

} // namespace core::application::analysis
