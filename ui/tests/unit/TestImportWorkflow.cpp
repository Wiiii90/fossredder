/**
 * @file ui/tests/unit/TestImportWorkflow.cpp
 * @brief Tests for import workflow run restoration and refresh behavior.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/import/ImportLog.h"
#include "core/errors/IErrorReporter.h"
#include "ui/workflows/import/ImportWorkflow.h"

namespace {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

core::application::workspace::WorkspaceSessionState makeStateWithImportLog(
    const QString& id,
    const QString& statementId = QString())
{
    core::application::workspace::WorkspaceSessionState state;
    auto log = std::make_shared<core::application::importing::ImportLog>();
    log->id = id.toStdString();
    log->time = "2026-05-15 10:00:00";
    log->type = "statement";
    log->file = "/tmp/import.pdf";
    log->status = "Success";
    log->message = "done";
    log->draftAttached = false;
    log->draftId.clear();
    log->statementId = statementId.toStdString();
    state.workflow.importLogs.push_back(std::move(log));
    return state;
}

} // namespace

namespace ui {

TEST(ImportWorkflowTest, RestoresPersistedRunsFromSnapshotProvider)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithImportLog(QStringLiteral("log-1"))]() {
        return state;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-1"));
    EXPECT_EQ(workflow.runs()->at(0).file, QStringLiteral("/tmp/import.pdf"));
    EXPECT_EQ(workflow.runs()->at(0).status, QStringLiteral("Success"));
}

TEST(ImportWorkflowTest, RefreshFromStateSnapshotReplacesRunRows)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithImportLog(QStringLiteral("log-1"))]() {
        return state;
    });
    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);

    const auto nextState = []() {
        core::application::workspace::WorkspaceSessionState next;
        auto first = std::make_shared<core::application::importing::ImportLog>();
        first->id = "log-2";
        first->time = "2026-05-15 11:00:00";
        first->type = "statement";
        first->file = "/tmp/other.pdf";
        first->status = "Failed";
        next.workflow.importLogs.push_back(first);

        auto second = std::make_shared<core::application::importing::ImportLog>();
        second->id = "log-3";
        second->time = "2026-05-15 12:00:00";
        second->type = "statement";
        second->file = "/tmp/third.pdf";
        second->status = "Draft";
        next.workflow.importLogs.push_back(second);
        return next;
    }();

    workflow.setStateSnapshotProvider([state = nextState]() {
        return state;
    });

    EXPECT_EQ(workflow.runs()->rowCount(), 2);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-2"));
    EXPECT_EQ(workflow.runs()->at(1).logId, QStringLiteral("log-3"));
}

TEST(ImportWorkflowTest, SettingTheLogStoreDoesNotOverwriteRestoredRuns)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithImportLog(QStringLiteral("log-1"))]() {
        return state;
    });

    int callCount = 0;
    workflow.setImportLogsStore([&callCount](const std::vector<core::application::importing::ImportLog>&) {
        ++callCount;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    EXPECT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("log-1"));
    EXPECT_EQ(callCount, 0);

    workflow.addRunNote(QStringLiteral("Success"), QStringLiteral("extra"));
    EXPECT_EQ(callCount, 1);
}

} // namespace ui
