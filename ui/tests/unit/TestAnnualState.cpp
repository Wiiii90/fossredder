/**
 * @file ui/tests/unit/TestAnnualState.cpp
 * @brief Tests for the UI AnnualState boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/annual/AnnualService.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/domain/entities/Analysis.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/state/session/AnnualState.h"
#include "ui/workflows/annual/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

struct AnnualStateHarness {
    tests::support::FakeStorageManager* storage = nullptr;
    std::unique_ptr<core::application::WorkspaceFacade> coreFacade;
    std::unique_ptr<WorkspaceFacade> facade;
    std::unique_ptr<AnnualWorkflow> workflow;
    std::unique_ptr<AnnualState> state;
};

auto makeAnnualCatalog()
{
    auto catalog = tests::support::makeWorkspaceCatalog();
    auto tabular = tests::support::makeAnalysis();
    tabular->setId("analysis-table");
    tabular->rename("Table");
    tabular->setType("tabular");
    tabular->setExportFormat("csv");
    tabular->setSnapshotTransactionsJson(
        R"([{"id":"tx-1","name":"Rent","bookingDate":"2026-01-05","amount":1250.0,"allocatable":true,"contractId":"contract-1","statementId":"statement-1","propertyIds":["property-1"]}])");

    auto plot = tests::support::makeAnalysis();
    plot->setId("analysis-plot");
    plot->rename("Plot");
    plot->setType("plot");
    plot->setExportFormat("png");
    plot->setSnapshotTransactionsJson(
        R"([{"id":"tx-2","name":"Fees","bookingDate":"2026-01-06","amount":-35.5,"allocatable":false,"contractId":"","statementId":"statement-1","propertyIds":["property-1"]}])");

    auto annual = tests::support::makeAnnual();
    annual->rename("Annual 2026");
    annual->setAnalysisIds({"analysis-table"});

    catalog.setAnalyses({tabular, plot});
    catalog.setAnnuals({annual});
    return catalog;
}

AnnualStateHarness makeHarness(bool selectAnnual)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade =
        std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    auto facade = std::make_unique<WorkspaceFacade>(coreFacade.get());

    auto catalog = makeAnnualCatalog();
    if (!selectAnnual) {
        catalog.setAnnuals({});
    }
    storagePtr->loadedState_.catalog = catalog;
    coreFacade->openFile("annual-state.fr");
    facade->setSelectedAnnualId(selectAnnual ? QStringLiteral("annual-1")
                                             : QString());

    auto service = std::make_shared<core::application::annual::AnnualService>();
    auto* coreFacadePtr = coreFacade.get();
    auto workflow = std::make_unique<AnnualWorkflow>(
        [coreFacadePtr]() { return coreFacadePtr->workspaceSnapshot(); }, service);

    auto state = std::make_unique<AnnualState>();
    state->setWorkspace(facade.get());
    state->setAnnualWorkflow(workflow.get());

    return {storagePtr, std::move(coreFacade), std::move(facade),
            std::move(workflow), std::move(state)};
}

bool sectionVisible(const QVariantList& sections, const QString& key)
{
    for (const QVariant& value : sections) {
        const QVariantMap section = value.toMap();
        if (section.value(QStringLiteral("key")).toString() == key) {
            return section.value(QStringLiteral("visible")).toBool();
        }
    }
    return false;
}

} // namespace

TEST(AnnualStateTest, ANN_ST_001_LoadsSelectedAnnualAndDerivedPanels)
{
    auto harness = makeHarness(true);

    EXPECT_TRUE(harness.state->isEdit());
    EXPECT_EQ(harness.state->name(), QStringLiteral("Annual 2026"));
    EXPECT_EQ(harness.state->year(), 2026);
    EXPECT_EQ(harness.state->assignedAnalysisRows().size(), 1);
    EXPECT_EQ(harness.state->availableAnalysisRows().size(), 1);
    EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                               QStringLiteral("deduplicated")));
    EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                               QStringLiteral("workspaceOnly")));
}

TEST(AnnualStateTest, ANN_ST_002_AssignmentChangesUpdatePreviewAndPersistAnnual)
{
    auto harness = makeHarness(true);

    harness.state->addAvailableAnalysisAtIndex(0);

    EXPECT_TRUE(harness.state->hasChanges());
    EXPECT_EQ(harness.state->analysisIds().size(), 2);
    EXPECT_TRUE(sectionVisible(harness.state->transactionSections(),
                               QStringLiteral("divergent")));
    EXPECT_FALSE(sectionVisible(harness.state->transactionSections(),
                                QStringLiteral("workspaceOnly")));

    harness.state->submitUpdate();
    const auto snapshot = harness.coreFacade->workspaceSnapshot();
    ASSERT_EQ(snapshot.annuals.size(), 1U);
    ASSERT_EQ(snapshot.annuals.front().analysisIds.size(), 2U);
    EXPECT_EQ(snapshot.annuals.front().analysisIds.at(1), std::string("analysis-plot"));
}

TEST(AnnualStateTest, ANN_ST_003_CreateStatePersistsNewAnnualThroughWorkspace)
{
    auto harness = makeHarness(false);

    harness.state->setName(QStringLiteral("Created Annual"));
    harness.state->setYear(2026);
    harness.state->addAvailableAnalysisAtIndex(0);
    harness.state->submitCreate();

    const auto snapshot = harness.coreFacade->workspaceSnapshot();
    ASSERT_EQ(snapshot.annuals.size(), 1U);
    EXPECT_EQ(snapshot.annuals.front().name, std::string("Created Annual"));
    EXPECT_EQ(snapshot.annuals.front().year, 2026);
    ASSERT_EQ(snapshot.annuals.front().analysisIds.size(), 1U);
    EXPECT_EQ(snapshot.annuals.front().analysisIds.front(),
              std::string("analysis-table"));
    EXPECT_FALSE(harness.facade->selectedAnnualId().isEmpty());
}

TEST(AnnualStateTest, ANN_ST_004_ExportFormatChangesRouteThroughWorkspace)
{
    auto harness = makeHarness(true);

    harness.state->setAnalysisExportFormat(QStringLiteral("analysis-plot"),
                                           QStringLiteral("JPG"));

    const auto snapshot = harness.coreFacade->workspaceSnapshot();
    ASSERT_EQ(snapshot.analyses.size(), 2U);
    EXPECT_EQ(snapshot.analyses.at(1).exportFormat, std::string("jpg"));
    EXPECT_TRUE(harness.state->hasChanges());
}

} // namespace ui
