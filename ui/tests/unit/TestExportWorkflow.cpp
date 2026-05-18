/**
 * @file ui/tests/unit/TestExportWorkflow.cpp
 * @brief Tests for export workflow run restoration and refresh behavior.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/export/ExportLog.h"
#include "core/errors/IErrorReporter.h"
#include "ui/workflows/export/ExportWorkflow.h"

namespace {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

core::application::workspace::WorkspaceSessionState makeStateWithExportLog(
    const QString& id)
{
    core::application::workspace::WorkspaceSessionState state;
    auto log = std::make_shared<core::application::exporting::ExportLog>();
    log->id = id.toStdString();
    log->time = "2026-05-15 10:00:00";
    log->targetPath = "/tmp/export.xlsx";
    log->status = "Success";
    log->message = "done";
    log->payload = "{}";
    state.workflow.exportLogs.push_back(std::move(log));
    return state;
}

} // namespace

namespace ui {

TEST(ExportWorkflowTest, RestoresPersistedRunsFromSnapshotProvider)
{
    auto runner = std::make_shared<ui::exporting::ExportRunner>();
    ExportWorkflow workflow([state = makeStateWithExportLog(QStringLiteral("log-1"))]() {
        return std::make_shared<const core::application::workspace::WorkspaceSessionState>(state);
    }, runner, {}, nullptr);

    workflow.setExportLogsStore([](const std::vector<core::application::exporting::ExportLog>&) {});
    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-1"));
    EXPECT_EQ(workflow.runs()->at(0).file, QStringLiteral("/tmp/export.xlsx"));
    EXPECT_EQ(workflow.runs()->at(0).status, QStringLiteral("Success"));
}

TEST(ExportWorkflowTest, RefreshFromStateSnapshotReplacesRunRows)
{
    auto runner = std::make_shared<ui::exporting::ExportRunner>();
    ExportWorkflow workflow([state = makeStateWithExportLog(QStringLiteral("log-1"))]() {
        return std::make_shared<const core::application::workspace::WorkspaceSessionState>(state);
    }, runner, {}, nullptr);

    workflow.setExportLogsStore([](const std::vector<core::application::exporting::ExportLog>&) {});
    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);

    workflow.refreshFromStateSnapshot();
    EXPECT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-1"));
}

TEST(ExportWorkflowTest, SettingTheLogStoreDoesNotOverwriteRestoredRuns)
{
    auto runner = std::make_shared<ui::exporting::ExportRunner>();
    ExportWorkflow workflow([state = makeStateWithExportLog(QStringLiteral("log-1"))]() {
        return std::make_shared<const core::application::workspace::WorkspaceSessionState>(state);
    }, runner, {}, nullptr);

    int callCount = 0;
    workflow.setExportLogsStore([&callCount](const std::vector<core::application::exporting::ExportLog>&) {
        ++callCount;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-1"));
    EXPECT_EQ(callCount, 0);

    workflow.removeRunAt(0);
    EXPECT_EQ(callCount, 1);
}

} // namespace ui
