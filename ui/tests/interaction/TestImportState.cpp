/**
 * @file ui/tests/interaction/TestImportState.cpp
 * @brief Interaction tests for `ImportState`.
 */

#include "gtest/gtest.h"

#include <QRegularExpression>

#include "ui/config/Defaults.h"
#include "ui/import/ImportState.h"
#include "ui/models/ImportRunList.h"
#include "ui/text/Text.h"

namespace {

using ui::ImportRunList;
using ui::importing::ImportState;

TEST(ImportStateTests, AddFilesUsesFirstFileAsSelectionAndQueuesRemainingFiles)
{
    ImportState state;

    const bool changed = state.addFiles({QStringLiteral(" first.pdf "), QString(), QStringLiteral("second.pdf"), QStringLiteral("third.pdf")});

    EXPECT_TRUE(changed);
    EXPECT_EQ(state.selectedFile(), QStringLiteral("first.pdf"));
    ASSERT_EQ(state.queuedFiles().size(), 2);
    EXPECT_EQ(state.queuedFiles().at(0), QStringLiteral("second.pdf"));
    EXPECT_EQ(state.queuedFiles().at(1), QStringLiteral("third.pdf"));
}

TEST(ImportStateTests, UpdateProgressExtractsPageInformationFromPhaseText)
{
    ImportState state;
    const QRegularExpression pagePattern(ui::config::kImportProgressPagePattern);

    state.beginImport(QStringLiteral("statement.pdf"));
    state.updateProgress(0.42, QStringLiteral("Parsing [3 / 7]"), pagePattern);

    EXPECT_DOUBLE_EQ(state.progress(), 0.42);
    EXPECT_EQ(state.phase(), QStringLiteral("Parsing [3 / 7]"));
    EXPECT_EQ(state.currentPage(), 3);
    EXPECT_EQ(state.pageCount(), 7);
}

TEST(ImportStateTests, RecordCanceledAppendsCanceledRunAndClearsCurrentSelection)
{
    ImportRunList runs;
    ImportState state;

    state.beginImport(QStringLiteral("statement.pdf"));
    state.beginCancel(true);
    state.recordCanceled(QStringLiteral("2025-01-15T12:00:00Z"));

    EXPECT_FALSE(state.isRunning());
    EXPECT_FALSE(state.cancelRequested());
    EXPECT_TRUE(state.selectedFile().isEmpty());
    EXPECT_TRUE(state.currentRunFile().isEmpty());
    EXPECT_EQ(runs.rowCount(), 0);
}

TEST(ImportStateTests, RecordFailedClearsQueueAndPersistsFailureMessage)
{
    ImportRunList runs;
    ImportState state;

    ASSERT_TRUE(state.addFiles({QStringLiteral("statement.pdf"), QStringLiteral("queued.pdf")}));
    const QString selected = state.takeSelectedFileForStart();
    ASSERT_EQ(selected, QStringLiteral("statement.pdf"));

    state.beginImport(selected);
    state.recordFailed(QStringLiteral("2025-01-15T12:00:00Z"), QStringLiteral("import failed"));

    EXPECT_FALSE(state.isRunning());
    EXPECT_EQ(state.error(), QStringLiteral("import failed"));
    EXPECT_TRUE(state.queuedFiles().isEmpty());
    EXPECT_EQ(runs.rowCount(), 0);
}

} // namespace
