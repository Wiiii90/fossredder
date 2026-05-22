/**
 * @file ui/tests/unit/TestImportWorkflow.cpp
 * @brief Tests for import workflow run restoration and refresh behavior.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/application/import/ImportLog.h"
#include "core/domain/entities/Statement.h"
#include "core/errors/IErrorReporter.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"

namespace {

class NoopErrorReporter final : public core::errors::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

class WriterSpy final : public core::ports::workspace::IWorkspaceWriter {
public:
    core::ports::workspace::StatementDraftCommand lastSavedDraft;

    void setSnapshotChangedCallback(SnapshotChanged) override {}
    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter>) override {}
    void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave) override {}
    void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad) override {}
    void setDeletionImpactCallback(core::ports::storage::IStorageManager::DeletionImpactCallback) override {}
    void openLatest() override {}
    void newFile(const std::string&) override {}
    void openFile(const std::string&) override {}
    void saveFile() override {}
    void saveFileAs(const std::string&) override {}
    void commit() override {}
    void notifySnapshot() override {}
    std::string addActor(const core::ports::workspace::ActorCommand&) override { return {}; }
    void updateActor(const core::ports::workspace::ActorCommand&) override {}
    void deleteActor(const std::string&) override {}
    std::string addProperty(const core::ports::workspace::PropertyCommand&) override { return {}; }
    void updateProperty(const core::ports::workspace::PropertyCommand&) override {}
    void deleteProperty(const std::string&) override {}
    std::string addContract(const core::ports::workspace::ContractCommand&) override { return {}; }
    void updateContract(const core::ports::workspace::ContractCommand&) override {}
    void deleteContract(const std::string&) override {}
    std::string addStatement(const core::ports::workspace::StatementCommand&) override { return {}; }
    void updateStatement(const core::ports::workspace::StatementCommand&) override {}
    void deleteStatement(const std::string&) override {}
    std::string addTransaction(const core::ports::workspace::TransactionCommand&) override { return {}; }
    void updateTransaction(const core::ports::workspace::TransactionCommand&) override {}
    void deleteTransaction(const std::string&) override {}
    std::string addAnalysis(const core::ports::workspace::AnalysisCommand&) override { return {}; }
    void updateAnalysis(const core::ports::workspace::AnalysisCommand&) override {}
    void deleteAnalysis(const std::string&) override {}
    std::string addAnnual(const core::ports::workspace::AnnualCommand&) override { return {}; }
    void updateAnnual(const core::ports::workspace::AnnualCommand&) override {}
    void deleteAnnual(const std::string&) override {}
    std::string finalizeStatementDraft(const core::ports::workspace::FinalizeStatementDraftCommand&) override { return {}; }
    void saveStatementDraft(const core::ports::workspace::StatementDraftCommand& command) override { lastSavedDraft = command; }
    void clearStatementDraft(const std::string&) override {}
    void setImportLogs(const core::ports::workspace::ImportLogsCommand&) override {}
    void setExportLogs(const core::ports::workspace::ExportLogsCommand&) override {}
};

class MatcherSpy final
    : public core::application::importing::draft::IImportMatcherService {
public:
    core::application::importing::draft::ImportMatcherPresentation buildImportSuggestions(
        const core::domain::catalog::WorkspaceCatalog&,
        const core::application::importing::draft::TransactionDraft&) const override
    {
        return {};
    }

    core::application::importing::draft::DraftTextSignals buildDraftTextSignals(
        const core::domain::catalog::WorkspaceCatalog&,
        const core::application::importing::draft::TransactionDraft&) const override
    {
        return {};
    }

    core::application::importing::draft::DraftDerivedState buildDraftDerivedState(
        const core::domain::catalog::WorkspaceCatalog&,
        const core::application::importing::draft::DraftLinkSelection&) const override
    {
        return {};
    }

    std::string resolveActorId(const core::domain::catalog::WorkspaceCatalog&,
                               const std::string&) const override
    {
        return {};
    }

    std::string resolveContractId(const core::domain::catalog::WorkspaceCatalog&,
                                  const std::string&) const override
    {
        return {};
    }

    bool contractIsFullyAllocatable(const core::domain::catalog::WorkspaceCatalog&,
                                    const std::string&) const override
    {
        return false;
    }

    core::domain::catalog::WorkspaceCatalog mergeCatalogState(
        core::domain::catalog::WorkspaceCatalog primary,
        const core::domain::catalog::WorkspaceCatalog&) const override
    {
        return primary;
    }

    std::vector<std::string> referenceAliasesFromMetadata(
        const std::string&) const override
    {
        return {};
    }
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

core::application::workspace::WorkspaceSessionState makeStateWithImportedStatement()
{
    auto state = makeStateWithImportLog(QStringLiteral("log-1"), QStringLiteral("statement-1"));
    auto statement = std::make_shared<core::domain::Statement>();
    statement->setId("statement-1");
    statement->rename("Imported statement");
    state.catalog.setStatements({statement});
    return state;
}

core::application::workspace::WorkspaceSessionState makeStateWithLegacyDraftLog()
{
    core::application::workspace::WorkspaceSessionState state;
    auto log = std::make_shared<core::application::importing::ImportLog>();
    log->id = "import-log-1";
    log->time = "2026-05-15 10:00:00";
    log->type = "statement";
    log->file = "/tmp/import.pdf";
    log->status = "Draft";
    log->message = "draft";
    log->draftAttached = false;
    log->draftId.clear();
    log->statementDraftIds = {"statement-draft-1"};
    state.workflow.importLogs.push_back(std::move(log));
    return state;
}

core::application::workspace::WorkspaceSessionState makeStateWithDraftStack()
{
    core::application::workspace::WorkspaceSessionState state;
    for (int i = 1; i <= 3; ++i) {
        const auto id = std::string("draft-") + std::to_string(i);

        auto statementDraft = std::make_shared<core::application::importing::draft::StatementDraft>();
        statementDraft->id = id;
        statementDraft->name = std::string("Draft ") + std::to_string(i);
        state.workflow.statementDrafts.push_back(statementDraft);

        for (int position = 0; position < 2; ++position) {
            auto transactionDraft = std::make_shared<core::application::importing::draft::TransactionDraft>();
            transactionDraft->id = std::string("tx-") + std::to_string(i) + "-" + std::to_string(position);
            transactionDraft->statementDraftId = id;
            transactionDraft->position = position;
            transactionDraft->name = std::string("Tx ") + std::to_string(i);
            transactionDraft->metadata = std::string("metadata-") + std::to_string(i);
            transactionDraft->proofImageData = {static_cast<std::uint8_t>(i)};
            state.workflow.transactionDrafts.push_back(transactionDraft);
        }

        auto log = std::make_shared<core::application::importing::ImportLog>();
        log->id = id;
        log->time = "2026-05-15 10:00:00";
        log->type = "statement";
        log->file = "/tmp/import.pdf";
        log->status = "Draft";
        log->draftAttached = true;
        log->draftId = id;
        state.workflow.importLogs.push_back(log);
    }
    return state;
}

core::application::workspace::WorkspaceSessionState makeStateWithTwoDraftRuns()
{
    core::application::workspace::WorkspaceSessionState state;

    auto first = std::make_shared<core::application::importing::ImportLog>();
    first->id = "draft-1";
    first->time = "2026-05-15 10:00:00";
    first->type = "statement";
    first->file = "/tmp/january.pdf";
    first->status = "Draft";
    first->message = "Draft ready";
    first->draftAttached = true;
    first->draftId = "draft-1";
    state.workflow.importLogs.push_back(first);

    auto second = std::make_shared<core::application::importing::ImportLog>();
    second->id = "draft-2";
    second->time = "2026-05-15 11:00:00";
    second->type = "statement";
    second->file = "/tmp/march.pdf";
    second->status = "Draft";
    second->message = "Draft ready";
    second->draftAttached = true;
    second->draftId = "draft-2";
    state.workflow.importLogs.push_back(second);

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

TEST(ImportWorkflowTest, MarksImportedRunDeletedWhenStatementIsMissing)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithImportLog(
                                           QStringLiteral("log-1"),
                                           QStringLiteral("statement-1"))]() {
        return state;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    ASSERT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).status, QStringLiteral("Deleted"));
    EXPECT_EQ(workflow.runs()->at(0).message,
              QStringLiteral("Imported statement was deleted."));
    EXPECT_TRUE(workflow.runs()->at(0).statementId.isEmpty());
}

TEST(ImportWorkflowTest, KeepsImportedRunNavigableWhenStatementExists)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithImportedStatement()]() {
        return state;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    ASSERT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_EQ(workflow.runs()->at(0).status, QStringLiteral("Success"));
    EXPECT_EQ(workflow.runs()->at(0).statementId, QStringLiteral("statement-1"));
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

TEST(ImportWorkflowTest, RestoresDraftRowsFromStatementDraftIds)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithLegacyDraftLog()]() {
        return state;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    ASSERT_EQ(workflow.runs()->rowCount(), 1);
    EXPECT_TRUE(workflow.runs()->at(0).draftAttached);
    EXPECT_EQ(workflow.runs()->at(0).draftId, QStringLiteral("statement-draft-1"));
}

TEST(ImportWorkflowTest, DraftStackNavigationCyclesThroughImportHome)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);
    const auto state = makeStateWithDraftStack();
    workflow.setStateSnapshotProvider([state]() {
        return state;
    });

    EXPECT_TRUE(workflow.hasDraftStack());
    EXPECT_TRUE(workflow.openNextDraft());
    ASSERT_NE(workflow.draft(), nullptr);
    EXPECT_EQ(workflow.draft()->draftId(), QStringLiteral("draft-1"));
    workflow.clearDraft();

    ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-3")));
    ASSERT_NE(workflow.draft(), nullptr);
    EXPECT_TRUE(workflow.hasNextDraft());
    EXPECT_TRUE(workflow.openNextDraft());
    EXPECT_EQ(workflow.draft(), nullptr);

    EXPECT_TRUE(workflow.openNextDraft());
    ASSERT_NE(workflow.draft(), nullptr);
    EXPECT_EQ(workflow.draft()->draftId(), QStringLiteral("draft-1"));

    EXPECT_TRUE(workflow.hasPrevDraft());
    EXPECT_TRUE(workflow.openPrevDraft());
    EXPECT_EQ(workflow.draft(), nullptr);

    EXPECT_TRUE(workflow.openPrevDraft());
    ASSERT_NE(workflow.draft(), nullptr);
    EXPECT_EQ(workflow.draft()->draftId(), QStringLiteral("draft-3"));
}

TEST(ImportWorkflowTest, ReopenedDraftRestoresRememberedTransactionIndex)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);
    const auto state = makeStateWithDraftStack();
    workflow.setStateSnapshotProvider([state]() {
        return state;
    });

    ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
    ASSERT_NE(workflow.draft(), nullptr);
    workflow.draft()->setCurrentIndex(1);

    workflow.clearDraft();
    ASSERT_TRUE(workflow.openPersistedDraft(QStringLiteral("draft-1")));
    ASSERT_NE(workflow.draft(), nullptr);
    EXPECT_EQ(workflow.draft()->currentIndex(), 1);
}

TEST(ImportWorkflowTest, PersistedDraftSnapshotPositionsFollowCurrentOrder)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    auto matcher = std::make_shared<MatcherSpy>();
    WriterSpy writer;
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter,
        {},
        matcher,
        &writer);

    StatementDraft draft;
    draft.setDraftId(QStringLiteral("draft-1"));
    std::vector<TransactionDraft> rows(2);
    rows[0].name = QStringLiteral("tx-a");
    rows[1].name = QStringLiteral("tx-b");
    draft.setDrafts(std::move(rows));
    draft.setCurrentIndex(0);
    draft.insertTransactionAfterCurrent();

    workflow.persistStatementDraft(&draft);

    const auto& savedDraft = writer.lastSavedDraft.draft;
    ASSERT_EQ(savedDraft.transactions.size(), 3u);
    EXPECT_EQ(savedDraft.transactions[0].name, "tx-a");
    EXPECT_EQ(savedDraft.transactions[0].position, 0);
    EXPECT_EQ(savedDraft.transactions[1].position, 1);
    EXPECT_EQ(savedDraft.transactions[2].name, "tx-b");
    EXPECT_EQ(savedDraft.transactions[2].position, 2);
}

TEST(ImportWorkflowTest, FinalizedRunNoteUsesExplicitDraftContextOverActiveDraft)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    workflow.setStateSnapshotProvider([state = makeStateWithTwoDraftRuns()]() {
        return state;
    });

    ASSERT_NE(workflow.runs(), nullptr);
    ASSERT_EQ(workflow.runs()->rowCount(), 2);

    workflow.activateRunAt(0);
    workflow.addRunNote(QStringLiteral("Finalized"),
                        QStringLiteral("Draft was finalized into a statement."),
                        false,
                        QStringLiteral("statement-2"),
                        QStringLiteral("draft-2"));

    EXPECT_EQ(workflow.runs()->at(0).logId, QStringLiteral("draft-1"));
    EXPECT_EQ(workflow.runs()->at(0).status, QStringLiteral("Draft"));
    EXPECT_TRUE(workflow.runs()->at(0).statementId.isEmpty());

    EXPECT_EQ(workflow.runs()->at(1).logId, QStringLiteral("draft-2"));
    EXPECT_EQ(workflow.runs()->at(1).status, QStringLiteral("Finalized"));
    EXPECT_EQ(workflow.runs()->at(1).statementId, QStringLiteral("statement-2"));
}

TEST(ImportWorkflowTest, PropertySelectionClearsSelectedContract)
{
    auto reporter = std::make_shared<NoopErrorReporter>();
    ImportWorkflow workflow(
        []() { return std::shared_ptr<core::jobs::JobSystem>{}; },
        reporter);

    StatementDraft draft;
    std::vector<TransactionDraft> rows(1);
    rows[0].contractId = QStringLiteral("contract-1");
    rows[0].contractSelected = true;
    draft.setDrafts(std::move(rows));
    draft.setCurrentIndex(0);

    workflow.setCurrentPropertySelected(&draft, QStringLiteral("property-1"), true);

    const auto current = draft.current();
    EXPECT_EQ(current.value(QStringLiteral("contractId")).toString(), QString());
    EXPECT_FALSE(current.value(QStringLiteral("contractSelected")).toBool());
}

TEST(StatementDraftViewModelTest, TransactionNavigationWrapsAtBothEdges)
{
    StatementDraft draft;
    std::vector<TransactionDraft> rows(3);
    draft.setDrafts(std::move(rows));

    draft.setCurrentIndex(2);
    draft.next();
    EXPECT_EQ(draft.currentIndex(), 0);

    draft.prev();
    EXPECT_EQ(draft.currentIndex(), 2);
}

} // namespace ui
