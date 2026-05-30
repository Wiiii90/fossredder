/**
 * @file ui/tests/unit/TestAnnualWorkflow.cpp
 * @brief Tests for the UI annual workflow boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/annual/AnnualService.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/domain/entities/Analysis.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/workflows/annual/AnnualWorkflow.h"

namespace ui {

namespace {

core::ports::workspace::WorkspaceSnapshot makeAnnualSnapshot()
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    core::application::WorkspaceFacade facade(std::move(storage));

    auto catalog = tests::support::makeWorkspaceCatalog();
    auto tabular = tests::support::makeAnalysis();
    tabular->setId("analysis-table");
    tabular->rename("Table");
    tabular->setType("tabular");
    tabular->setSnapshotTransactionsJson(
        R"([{"id":"tx-1","name":"Rent","bookingDate":"2026-01-05","amount":1250.0,"allocatable":true,"contractId":"contract-1","statementId":"statement-1","propertyIds":["property-1"]}])");

    auto plot = tests::support::makeAnalysis();
    plot->setId("analysis-plot");
    plot->rename("Plot");
    plot->setType("plot");
    plot->setSnapshotTransactionsJson(
        R"([{"id":"tx-2","name":"Fees","bookingDate":"2026-01-06","amount":-35.5,"allocatable":false,"contractId":"","statementId":"statement-1","propertyIds":["property-1"]}])");

    auto annual = tests::support::makeAnnual();
    annual->setAnalysisIds({"analysis-table"});
    catalog.setAnalyses({tabular, plot});
    catalog.setAnnuals({annual});

    storagePtr->loadedState_.catalog = catalog;
    facade.openFile("annual-workflow.fr");
    return facade.workspaceSnapshot();
}

} // namespace

TEST(AnnualWorkflowTest, ANN_W_001_ComputeAnnualUsesStoredAnnualAssignments)
{
    const auto snapshot = makeAnnualSnapshot();
    auto annualService =
        std::make_shared<core::application::annual::AnnualService>();
    AnnualWorkflow workflow([snapshot]() { return snapshot; }, annualService);

    const QVariantMap result = workflow.computeAnnual(QStringLiteral("annual-1"));
    const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();

    EXPECT_EQ(result.value(QStringLiteral("annualId")).toString(),
              QStringLiteral("annual-1"));
    EXPECT_EQ(stats.value(QStringLiteral("assignedAnalysisCount")).toInt(), 1);
    EXPECT_EQ(result.value(QStringLiteral("deduplicated")).toList().size(), 1);
    EXPECT_EQ(result.value(QStringLiteral("workspaceOnly")).toList().size(), 1);
}

TEST(AnnualWorkflowTest, ANN_W_002_ComputeAnnualPreviewUsesInjectedAnalysisIdsAndYear)
{
    const auto snapshot = makeAnnualSnapshot();
    auto annualService =
        std::make_shared<core::application::annual::AnnualService>();
    AnnualWorkflow workflow([snapshot]() { return snapshot; }, annualService);

    const QVariantMap result = workflow.computeAnnualPreview(
        QStringLiteral("annual-1"), {QStringLiteral("analysis-table"),
                                      QStringLiteral("analysis-plot")},
        2026);
    const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();

    EXPECT_EQ(stats.value(QStringLiteral("assignedAnalysisCount")).toInt(), 2);
    EXPECT_EQ(result.value(QStringLiteral("divergent")).toList().size(), 2);
    EXPECT_EQ(result.value(QStringLiteral("workspaceOnly")).toList().size(), 0);
}

} // namespace ui
