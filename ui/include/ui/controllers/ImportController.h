/**
 * @file ui/include/ui/controllers/ImportController.h
 * @brief Declares the import workflow controller exposed to QML.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <qqmlintegration.h>
#include <exception>
#include <functional>
#include <memory>

#include "core/errors/IErrorReporter.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"
#include "core/models/AppState.h"
#include "core/models/ImportLog.h"
#include "core/models/StatementDraft.h"
#include "ui/import/ImportJobBridge.h"
#include "ui/import/ImportState.h"
#include "ui/models/ImportRunList.h"
#include "ui/models/StatementDraft.h"

namespace core::jobs { class JobSystem; }

namespace ui {

/**
 * @brief Coordinates asynchronous statement imports, progress reporting, and draft creation for QML.
 */
class ImportController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ImportController)
    QML_UNCREATABLE("ImportController is provided by the application context")

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
    Q_PROPERTY(QString phase READ phase NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY stateChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY stateChanged)
    Q_PROPERTY(QString selectedFile READ selectedFile WRITE setSelectedFile NOTIFY stateChanged)
    Q_PROPERTY(int queuedCount READ queuedCount NOTIFY stateChanged)
    Q_PROPERTY(QStringList queuedFiles READ queuedFiles NOTIFY stateChanged)
    Q_PROPERTY(ImportRunList* runs READ runs CONSTANT)
    Q_PROPERTY(StatementDraft* draft READ draft NOTIFY stateChanged)
    Q_PROPERTY(bool hasPrevDraft READ hasPrevDraft NOTIFY stateChanged)
    Q_PROPERTY(bool hasNextDraft READ hasNextDraft NOTIFY stateChanged)

public:
    using JobSystemFactory = std::function<std::shared_ptr<core::jobs::JobSystem>()>;
    using StateSnapshotProvider = std::function<core::domain::AppState()>;
    using ImportLogsStore = std::function<void(const std::vector<core::domain::ImportLog>&)>;
    using StatementDraftStore = std::function<void(const core::domain::StatementDraft&)>;

    explicit ImportController(JobSystemFactory jobSystemFactory,
                              std::shared_ptr<core::errors::IErrorReporter> errorReporter,
                              QObject* parent = nullptr);

    /** @brief Provide a snapshot callback used when creating UI drafts from import results.
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
    void setSelectedFile(const QString& path);
    int queuedCount() const noexcept { return state_.queuedFiles().size(); }
    QStringList queuedFiles() const;
    StatementDraft* draft() const noexcept;
    bool hasPrevDraft() const;
    bool hasNextDraft() const;

    /** @brief Start importing the selected file or the next queued file. */
    Q_INVOKABLE void startStatementImport();

    /** @brief Append files to the import selection and queue.
     *  @param paths File paths to add
     */
    Q_INVOKABLE void addFiles(const QStringList& paths);

    /** @brief Clear transient import status once no import is running. */
    Q_INVOKABLE void resetStatus();

    /** @brief Clear the current statement draft and continue with queued imports if needed. */
    Q_INVOKABLE void clearDraft();

    /** @brief Reopen a persisted statement draft from workspace state when available. */
    Q_INVOKABLE bool openPersistedDraft(const QString& logId = {});

    /** @brief Indicates whether a persisted statement draft is currently available. */
    Q_INVOKABLE bool hasPersistedDraft() const;

    /** @brief Append a UI-side import run note for draft lifecycle actions. */
    Q_INVOKABLE void addRunNote(const QString& status, const QString& message, bool draftAttached = false,
                                const QString& statementId = {});
    Q_INVOKABLE void removeRunAt(int index);
    Q_INVOKABLE void activateRunAt(int index);
    Q_INVOKABLE bool openPrevDraft();
    Q_INVOKABLE bool openNextDraft();

    /** @brief Request cancellation of the active import. */
    Q_INVOKABLE void cancelImport();

    /** @brief Request cancellation of the active import and clear the remaining queue. */
    Q_INVOKABLE void cancelAllImports();

    /** @brief Return the model that tracks persisted import-run entries. */
    ImportRunList* runs() noexcept;

signals:
    void stateChanged();
    void importFinished();
    void importCanceled();
    void importFailed(const QString& error);

private slots:
    void updateProgress(double p, const QString& phase);
    void onJobTerminal(core::jobs::JobState state, const QString& message);

private:
    bool ensureJobBridge();
    void rejectImportStart(const QString& errorMessage, const char* traceMessage);
    void requestImportCancellation(bool clearQueue, const char* origin, const char* traceMessage);
    void handleJobEvent(const core::jobs::JobEvent& event);
    void handleImportCanceled(const QString& now);
    void handleImportFailed(const QString& now, const QString& errorMessage, const char* traceMessage);
    bool populateDraftFromResult(const QString& now);
    std::unique_ptr<ImportRunList> runs_;
    importing::ImportState state_;
    JobSystemFactory jobSystemFactory_;
    StateSnapshotProvider stateSnapshotProvider_;
    ImportLogsStore importLogsStore_;
    std::unique_ptr<importing::ImportJobBridge> jobBridge_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;

    void startNextQueuedImport();
    void startImportForFile(const QString& path);
    void reportException(const char* origin, std::exception_ptr exception) const;
    bool restoreDraftFromState(const core::domain::AppState& snapshot);
    QStringList draftStackIds() const;
    int activeDraftStackIndex() const;
    void restoreRunsFromSnapshot(const core::domain::AppState& snapshot);
    void persistRuns();
    QString resolveDraftContextLogId() const;
    ImportRunRow upsertRunById(const QString& logId,
                               const QString& status,
                               const QString& message,
                               bool draftAttached,
                               const QString& draftId = {},
                               const QString& statementId = {});
    ImportRunRow upsertActiveDraftRun(const QString& status, const QString& message, bool draftAttached,
                                      const QString& statementId = {});
    bool saveImportedDraft(const QString& draftId,
                           const std::shared_ptr<core::domain::Statement>& statement,
                           const std::vector<core::domain::TransactionDraft>& transactions) const;

    QString activeDraftLogId_;
    QString activeRunLogId_;
    QString activeRunDraftId_;
    bool activeRunTerminalHandled_ = false;
    StatementDraftStore statementDraftStore_;
};

} // namespace ui
