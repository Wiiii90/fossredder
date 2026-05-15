/**
 * @file ui/include/ui/workflows/import/ImportWorkflow.h
 * @brief Declares the import workflow exposed to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <exception>
#include <functional>
#include <memory>
#include <qqmlintegration.h>

#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/IImportMatcherService.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "core/errors/IErrorReporter.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"
#include "core/ports/presenters/IImportPresenter.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "ui/workflows/import/ImportJobBridge.h"
#include "ui/workflows/import/ImportWorkflowState.h"
#include "ui/viewmodels/import/ImportRunListModel.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/viewmodels/import/TransactionDraftViewModel.h"

namespace core::jobs {
class JobSystem;
}

namespace ui {

/**
 * @brief Coordinates asynchronous statement imports, progress reporting, and
 * draft creation for QML.
 */
class ImportWorkflow : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ImportWorkflow)
  QML_UNCREATABLE("ImportWorkflow is provided by the application context")

  Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
  Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
  Q_PROPERTY(QString phase READ phase NOTIFY stateChanged)
  Q_PROPERTY(QString error READ error NOTIFY stateChanged)
  Q_PROPERTY(int currentPage READ currentPage NOTIFY stateChanged)
  Q_PROPERTY(int pageCount READ pageCount NOTIFY stateChanged)
  Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY
                 stateChanged)
  Q_PROPERTY(int queuedCount READ queuedCount NOTIFY stateChanged)
  Q_PROPERTY(QStringList queuedFiles READ queuedFiles NOTIFY stateChanged)
  Q_PROPERTY(ImportRunList *runs READ runs CONSTANT)
  Q_PROPERTY(StatementDraft *draft READ draft NOTIFY stateChanged)
  Q_PROPERTY(bool hasPrevDraft READ hasPrevDraft NOTIFY stateChanged)
  Q_PROPERTY(bool hasNextDraft READ hasNextDraft NOTIFY stateChanged)

public:
  using JobSystemFactory =
      std::function<std::shared_ptr<core::jobs::JobSystem>()>;
  using StateSnapshotProvider =
      std::function<core::application::workspace::WorkspaceSessionState()>;
  using ImportLogsStore = std::function<void(
      const std::vector<core::application::importing::ImportLog> &)>;
  using StatementDraftStore = std::function<void(
      const core::application::importing::draft::StatementDraft &)>;

  explicit ImportWorkflow(
      JobSystemFactory jobSystemFactory,
      std::shared_ptr<core::errors::IErrorReporter> errorReporter,
      std::shared_ptr<core::ports::presenters::IImportPresenter>
          importPresenter = {},
      std::shared_ptr<
          core::application::importing::draft::IImportMatcherService>
          importMatcherService = {},
      core::ports::workspace::IWorkspaceWriter *workspaceWriter = nullptr,
      QObject *parent = nullptr);

  /** @brief Provide a snapshot callback used when creating UI drafts from
   * import results.
   *  @param provider Snapshot provider callback
   */
  void setStateSnapshotProvider(StateSnapshotProvider provider);
  void setImportLogsStore(ImportLogsStore store);
  void setStatementDraftStore(StatementDraftStore store);

  bool isRunning() const noexcept { return state_.isRunning(); }
  double progress() const noexcept { return state_.progress(); }
  QString phase() const { return state_.phase(); }
  QString error() const { return state_.error(); }
  int currentPage() const noexcept { return state_.currentPage(); }
  int pageCount() const noexcept { return state_.pageCount(); }
  QString selectedFile() const;
  void setSelectedFile(const QString &path);
  int queuedCount() const noexcept { return state_.queuedFiles().size(); }
  QStringList queuedFiles() const;
  StatementDraft *draft() const noexcept;
  bool hasPrevDraft() const;
  bool hasNextDraft() const;

  /** @brief Start importing the selected file or the next queued file. */
  Q_INVOKABLE void startStatementImport();

  /** @brief Append files to the import selection and queue.
   *  @param paths File paths to add
   */
  Q_INVOKABLE void addFiles(const QStringList &paths);

  /** @brief Clear transient import status once no import is running. */
  Q_INVOKABLE void resetStatus();

  /** @brief Clear the current statement draft and continue with queued imports
   * if needed. */
  Q_INVOKABLE void clearDraft();

  /** @brief Reopen a persisted statement draft from workspace state when
   * available. */
  Q_INVOKABLE bool openPersistedDraft(const QString &logId = {});

  /** @brief Indicates whether a persisted statement draft is currently
   * available. */
  Q_INVOKABLE bool hasPersistedDraft() const;

  /** @brief Append a UI-side import run note for draft lifecycle actions. */
  Q_INVOKABLE void addRunNote(const QString &status, const QString &message,
                              bool draftAttached = false,
                              const QString &statementId = {});
  Q_INVOKABLE void removeRunAt(int index);
  Q_INVOKABLE void activateRunAt(int index);
  Q_INVOKABLE bool openPrevDraft();
  Q_INVOKABLE bool openNextDraft();

  /** @brief Finalizes the current statement draft into persistent workspace state. */
  Q_INVOKABLE QString finalizeStatementDraft(StatementDraft *draft);
  Q_INVOKABLE void persistStatementDraft(StatementDraft *draft);
  Q_INVOKABLE void clearPersistedStatementDraft(const QString &draftId = {});
  Q_INVOKABLE QVariantMap currentTransactionViewState(StatementDraft *draft) const;
  Q_INVOKABLE QVariantMap findChoiceRowByText(const QVariantList &rows,
                                             const QString &text) const;
  Q_INVOKABLE void syncCurrentTransactionDraft(StatementDraft *draft);
  Q_INVOKABLE void selectCurrentActorChoice(StatementDraft *draft,
                                            const QVariantMap &row);
  Q_INVOKABLE void selectCurrentContractChoice(StatementDraft *draft,
                                               const QVariantMap &row);
  Q_INVOKABLE void setCurrentPropertySelected(StatementDraft *draft,
                                              const QString &propertyId,
                                              bool selected);
  Q_INVOKABLE void updateCurrentAmount(StatementDraft *draft,
                                       const QString &text);

  /** @brief Request cancellation of the active import. */
  Q_INVOKABLE void cancelImport();

  /** @brief Request cancellation of the active import and clear the remaining
   * queue. */
  Q_INVOKABLE void cancelAllImports();

  /** @brief Return the model that tracks persisted import-run entries. */
  ImportRunList *runs() noexcept;

signals:
  void stateChanged();
  void importFinished();
  void importCanceled();
  void importFailed(const QString &error);

private slots:
  void updateProgress(double p, const QString &phase);
  void onJobTerminal(core::jobs::JobState state, const QString &message);

private:
  bool ensureJobBridge();
  void rejectImportStart(const QString &errorMessage, const char *traceMessage);
  void requestImportCancellation(bool clearQueue, const char *origin,
                                 const char *traceMessage);
  void handleJobEvent(const core::jobs::JobEvent &event);
  void handleImportCanceled(const QString &now);
  void handleImportFailed(const QString &now, const QString &errorMessage,
                          const char *traceMessage);
  bool populateDraftFromResult(const QString &now);
  std::unique_ptr<ImportRunList> runs_;
  importing::ImportWorkflowState state_;
  JobSystemFactory jobSystemFactory_;
  StateSnapshotProvider stateSnapshotProvider_;
  ImportLogsStore importLogsStore_;
  std::unique_ptr<importing::ImportJobBridge> jobBridge_;
  std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
  std::shared_ptr<core::ports::presenters::IImportPresenter> importPresenter_;
  std::shared_ptr<core::application::importing::draft::IImportMatcherService>
      importMatcherService_;
  core::ports::workspace::IWorkspaceWriter *workspaceWriter_ = nullptr;

  void startNextQueuedImport();
  void startImportForFile(const QString &path);
  void reportException(const char *origin, std::exception_ptr exception) const;
  bool restoreDraftFromState(
      const core::application::workspace::WorkspaceSessionState &snapshot);
  QStringList draftStackIds() const;
  int activeDraftStackIndex() const;
  void restoreRunsFromSnapshot(
      const core::application::workspace::WorkspaceSessionState &snapshot);
  void persistRuns();
  QString resolveDraftContextLogId() const;
  ImportRunRow upsertRunById(const QString &logId, const QString &status,
                             const QString &message, bool draftAttached,
                             const QString &draftId = {},
                             const QString &statementId = {});
  ImportRunRow upsertActiveDraftRun(const QString &status,
                                    const QString &message, bool draftAttached,
                                    const QString &statementId = {});
  bool saveImportedDraft(
      const QString &draftId,
      const std::shared_ptr<core::domain::Statement> &statement,
      const std::vector<core::application::importing::draft::TransactionDraft>
          &transactions) const;
  core::domain::catalog::WorkspaceCatalog matchingStateForDraft(
      const StatementDraft *draft) const;
  core::application::importing::draft::StatementDraft buildFinalizationInput(
      StatementDraft *draft) const;
  void syncCurrentTransactionDraftImpl(StatementDraft *draft);

  QString activeDraftLogId_;
  QString activeRunLogId_;
  QString activeRunDraftId_;
  bool activeRunTerminalHandled_ = false;
  StatementDraftStore statementDraftStore_;
};

} // namespace ui

