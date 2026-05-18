/**
 * @file ui/tests/unit/TestWorkspaceStartupRehydration.cpp
 * @brief Tests for startup rehydration of persisted workspace state into UI-facing models.
 */

#include <gtest/gtest.h>

#include <memory>

#include <QVariantList>

#include "core/application/export/ExportLog.h"
#include "core/application/import/ImportLog.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/errors/IErrorReporter.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/workflows/export/ExportWorkflow.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace {

core::application::workspace::WorkspaceSessionState makeLoadedWorkspaceState()
{
    core::application::workspace::WorkspaceSessionState state;
    state.catalog = ui::tests::support::makeWorkspaceCatalog();

    auto importLog = std::make_shared<core::application::importing::ImportLog>();
    importLog->id = "import-log-1";
    importLog->time = "2026-05-15T08:00:00Z";
    importLog->type = "statement";
    importLog->file = "/tmp/import.csv";
    importLog->status = "done";
    importLog->message = "Import completed";
    importLog->draftAttached = true;
    importLog->draftId = "statement-draft-1";
    importLog->statementDraftIds = {"statement-draft-1"};
    importLog->statementId = "statement-1";
    state.workflow.importLogs.push_back(std::move(importLog));

    auto exportLog = std::make_shared<core::application::exporting::ExportLog>();
    exportLog->id = "export-log-1";
    exportLog->time = "2026-05-15T09:00:00Z";
    exportLog->targetPath = "/tmp/export.csv";
    exportLog->status = "done";
    exportLog->message = "Export completed";
    exportLog->payload = "{}";
    exportLog->annualIds = {"annual-1"};
    exportLog->analysisIds = {"analysis-1"};
    state.workflow.exportLogs.push_back(std::move(exportLog));

    return state;
}

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

} // namespace

namespace ui {

TEST(WorkspaceStartupRehydrationTest, RehydratesWorkspaceAndWorkflowModelsFromLoadedState)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    storage->loadedState_ = makeLoadedWorkspaceState();

    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    coreFacade->openFile("P:/workspace.db");

    WorkspaceFacade facade(coreFacade.get());

    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.properties()->rowCount(), 1);
    EXPECT_EQ(facade.contracts()->rowCount(), 1);
    EXPECT_EQ(facade.statements()->rowCount(), 1);
    EXPECT_EQ(facade.transactions()->rowCount(), 2);
    EXPECT_EQ(facade.analyses()->rowCount(), 1);
    EXPECT_EQ(facade.annuals()->rowCount(), 1);
    EXPECT_EQ(facade.statementTransactionIds(QStringLiteral("statement-1")),
              QVariantList({QStringLiteral("tx-1"), QStringLiteral("tx-2")}));

    ImportWorkflow importWorkflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        std::make_shared<NoopErrorReporter>(),
        std::shared_ptr<core::ports::presenters::IImportPresenter>{},
        std::shared_ptr<core::application::importing::draft::IImportMatcherService>{},
        nullptr);
    ExportWorkflow exportWorkflow(
        [coreFacade = coreFacade.get()]() {
            return std::make_shared<const core::application::workspace::WorkspaceSessionState>(coreFacade->state());
        },
        std::make_shared<ui::exporting::ExportRunner>(),
        {});

    importWorkflow.setStateSnapshotProvider([coreFacade = coreFacade.get()]() {
        return coreFacade->state();
    });
    exportWorkflow.setExportLogsStore([](const std::vector<core::application::exporting::ExportLog>&) {});
    exportWorkflow.refreshFromStateSnapshot();

    ASSERT_NE(importWorkflow.runs(), nullptr);
    ASSERT_NE(exportWorkflow.runs(), nullptr);
    EXPECT_EQ(importWorkflow.runs()->rowCount(), 1);
    EXPECT_EQ(exportWorkflow.runs()->rowCount(), 1);
    EXPECT_EQ(importWorkflow.runs()->at(0).logId, QStringLiteral("import-log-1"));
    EXPECT_EQ(exportWorkflow.runs()->at(0).logId, QStringLiteral("export-log-1"));
}

} // namespace ui
