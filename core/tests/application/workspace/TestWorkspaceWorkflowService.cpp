/**
 * @file core/tests/application/workspace/TestWorkspaceWorkflowService.cpp
 * @brief Tests for workspace workflow state handling.
 */

#include <gtest/gtest.h>

#include "core/application/workspace/WorkspaceSession.h"
#include "core/application/workspace/WorkspaceWorkflowService.h"
#include "application/workspace/TestWorkspaceSupport.h"

namespace core::application {

TEST(WorkspaceWorkflowServiceTest, SavesAndClearsStatementDraftState) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    WorkspaceSession session(std::move(storage));
    WorkspaceWorkflowService service(session);

    core::ports::workspace::StatementDraftCommand saveCommand;
    saveCommand.draft.id = "draft-1";
    saveCommand.draft.name = "Imported Statement";
    saveCommand.draft.transactionIds = {"tx-1"};
    saveCommand.draft.transactions.push_back({
        "tx-1",
        "draft-1",
        "Rent",
        "2024-01-31",
        {},
        10.0,
        {},
        {},
        {},
        static_cast<int>(core::domain::Transaction::Status::Neutral),
        true,
        0,
        {}
    });

    service.saveStatementDraft(saveCommand);

    EXPECT_EQ(session.state().workflow.statementDrafts.size(), 1u);
    EXPECT_EQ(session.state().workflow.transactionDrafts.size(), 1u);
    ASSERT_FALSE(session.state().workflow.statementDrafts.empty());
    EXPECT_EQ(session.state().workflow.statementDrafts.front()->id, "draft-1");

    service.clearStatementDraft("draft-1");

    EXPECT_TRUE(session.state().workflow.statementDrafts.empty());
    EXPECT_TRUE(session.state().workflow.transactionDrafts.empty());
}

TEST(WorkspaceWorkflowServiceTest, ReplacesImportAndExportLogsAtomically) {
    auto storage = std::make_unique<core::tests::application::workspace::FakeStorageManager>();
    WorkspaceSession session(std::move(storage));
    WorkspaceWorkflowService service(session);

    core::ports::workspace::ImportLogsCommand importLogs;
    importLogs.logs.push_back({"import-1", "2024-01-01T00:00:00Z", "csv", "file.csv", "done", "ok", false, {}, {}, {}});
    service.setImportLogs(importLogs);

    core::ports::workspace::ExportLogsCommand exportLogs;
    exportLogs.logs.push_back({"export-1", "2024-01-02T00:00:00Z", "target", "done", "ok", "payload", {}, {}});
    service.setExportLogs(exportLogs);

    EXPECT_EQ(session.state().workflow.importLogs.size(), 1u);
    EXPECT_EQ(session.state().workflow.exportLogs.size(), 1u);
    ASSERT_FALSE(session.state().workflow.importLogs.empty());
    ASSERT_FALSE(session.state().workflow.exportLogs.empty());
    EXPECT_EQ(session.state().workflow.importLogs.front()->id, "import-1");
    EXPECT_EQ(session.state().workflow.exportLogs.front()->id, "export-1");
}

} // namespace core::application
