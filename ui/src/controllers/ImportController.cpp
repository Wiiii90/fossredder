/**
 * @file ui/src/controllers/ImportController.cpp
 * @brief Implements the import workflow controller used by the QML UI.
 */

#include "ui/controllers/ImportController.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QUuid>

#include <exception>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>

#include "core/constants/CoreDefaults.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "core/models/Statement.h"
#include "core/models/ImportLog.h"
#include "ui/config/Defaults.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/import/ImportRunStore.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"
#include "ui/text/Text.h"

namespace ui {

namespace {

/** @brief Returns an ISO timestamp for import run bookkeeping. */
QString currentTimestamp()
{
    return QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
}

QString generateLogId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

ImportRunRow toRunRow(const std::shared_ptr<core::domain::ImportLog>& log)
{
    ImportRunRow row;
    if (!log) return row;
    row.logId = QString::fromStdString(log->id);
    row.time = QString::fromStdString(log->time);
    row.type = QString::fromStdString(log->type);
    row.file = QString::fromStdString(log->file);
    row.status = QString::fromStdString(log->status);
    row.message = QString::fromStdString(log->message);
    row.draftAttached = log->draftAttached;
    row.draftId = QString::fromStdString(log->draftId);
    row.statementId = QString::fromStdString(log->statementId);
    return row;
}

core::domain::ImportLog toImportLog(const ImportRunRow& row)
{
    core::domain::ImportLog log;
    log.id = strings::toStdString(row.logId);
    log.time = strings::toStdString(row.time);
    log.type = strings::toStdString(row.type);
    log.file = strings::toStdString(row.file);
    log.status = strings::toStdString(row.status);
    log.message = strings::toStdString(row.message);
    log.draftAttached = row.draftAttached;
    log.draftId = strings::toStdString(row.draftId);
    log.statementId = strings::toStdString(row.statementId);
    return log;
}

/** @brief Clamps import progress values to the configured UI range. */
double clampProgress(double progress)
{
    if (progress < ui::config::importProgress::kMinimum)
        return ui::config::importProgress::kMinimum;
    if (progress > ui::config::importProgress::kMaximum)
        return ui::config::importProgress::kMaximum;
    return progress;
}

/** @brief Builds the import job specification and prepares a fresh import run folder. */
core::jobs::ImportStatementJobSpec buildImportSpec(const QString& path,
                                                   QString& runRoot)
{
    importing::cleanupOldImportRuns(core::constants::importing::runs::kKeepCount);

    const auto runInfo = importing::createImportRunInfo();
    runRoot = runInfo.runRoot;

    core::jobs::ImportStatementJobSpec spec;
    spec.sourcePath = strings::toEncodedPath(path);
    spec.runRoot = strings::toEncodedPath(runInfo.runRoot);

    const QByteArray runIdNative = runInfo.runIdPrefix.toUtf8();
    spec.runIdPrefix = {runIdNative.constData(),
                        static_cast<size_t>(runIdNative.size())};
    return spec;
}

} // namespace

ImportController::ImportController(
    JobSystemFactory jobSystemFactory,
    std::shared_ptr<core::errors::IErrorReporter> errorReporter,
    QObject *parent)
    : QObject(parent), runs_(std::make_unique<ImportRunList>(this)),
      jobSystemFactory_(std::move(jobSystemFactory)),
      errorReporter_(std::move(errorReporter)) {
  if (!errorReporter_)
    throw std::invalid_argument("ImportController requires an error reporter");
}

void ImportController::setStateSnapshotProvider(StateSnapshotProvider provider) {
  stateSnapshotProvider_ = std::move(provider);
  if (stateSnapshotProvider_) {
    const auto snapshot = stateSnapshotProvider_();
    restoreRunsFromSnapshot(snapshot);
    emit stateChanged();
  }
}

void ImportController::setStatementDraftStore(StatementDraftStore store)
{
  statementDraftStore_ = std::move(store);
}

void ImportController::activateRunAt(int index)
{
  const auto row = runs_->at(index);
  if (row.logId.isEmpty() || !row.draftAttached) return;
  activeDraftLogId_ = !row.draftId.isEmpty() ? row.draftId : row.logId;
}

bool ImportController::openPrevDraft()
{
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  if (index <= 0 || index >= ids.size()) return false;
  return openPersistedDraft(ids.at(index - 1));
}

bool ImportController::openNextDraft()
{
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  if (index < 0 || index >= ids.size() - 1) return false;
  return openPersistedDraft(ids.at(index + 1));
}

void ImportController::setImportLogsStore(ImportLogsStore store)
{
  importLogsStore_ = std::move(store);
}

QString ImportController::selectedFile() const {
  return state_.selectedFile();
}

QStringList ImportController::queuedFiles() const {
  return state_.queuedFiles();
}

StatementDraft *ImportController::draft() const noexcept {
  return state_.draft();
}

bool ImportController::hasPrevDraft() const
{
  const auto index = activeDraftStackIndex();
  return index > 0;
}

bool ImportController::hasNextDraft() const
{
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  return index >= 0 && index < ids.size() - 1;
}

ImportRunList *ImportController::runs() noexcept {
  return runs_.get();
}

void ImportController::addRunNote(const QString& status, const QString& message, bool draftAttached,
                                  const QString& statementId) {
  if (draftAttached || !activeDraftLogId_.isEmpty() || !statementId.isEmpty()) {
    upsertActiveDraftRun(status, message, draftAttached, statementId);
  } else {
    runs_->addRun(currentTimestamp(),
                  ui::text::importRuns::typeStatement(),
                  state_.currentRunFile(),
                  status,
                  message,
                  false,
                  statementId,
                  generateLogId());
  }
  persistRuns();
  emit stateChanged();
}

void ImportController::removeRunAt(int index)
{
  const auto row = runs_->at(index);
  if (row.logId == activeDraftLogId_) {
    activeDraftLogId_.clear();
  }
  runs_->removeAt(index);
  persistRuns();
  emit stateChanged();
}

void ImportController::restoreRunsFromSnapshot(const core::domain::AppState& snapshot)
{
  std::vector<ImportRunRow> rows;
  rows.reserve(snapshot.importLogs.size());
  activeDraftLogId_.clear();

  for (const auto& item : snapshot.importLogs) {
    if (!item) continue;
    auto row = toRunRow(item);
    if (row.logId.isEmpty()) row.logId = generateLogId();
    if (row.draftAttached && row.draftId.isEmpty()) row.draftId = row.logId;
    if (row.draftAttached && activeDraftLogId_.isEmpty()) activeDraftLogId_ = row.logId;
    rows.push_back(std::move(row));
  }
  runs_->setRuns(std::move(rows));
}

void ImportController::persistRuns()
{
  if (!importLogsStore_) return;
  auto items = runs_->snapshot();
  bool normalized = false;
  for (auto& row : items) {
    if (!row.logId.isEmpty()) continue;
    row.logId = generateLogId();
    runs_->upsertRun(row);
    normalized = true;
  }
  if (normalized) {
    items = runs_->snapshot();
  }
  std::vector<core::domain::ImportLog> logs;
  logs.reserve(items.size());
  for (const auto& row : items) {
    logs.push_back(toImportLog(row));
  }
  importLogsStore_(logs);
}

QString ImportController::resolveDraftContextLogId() const
{
  if (!activeDraftLogId_.isEmpty()) return activeDraftLogId_;
  if (state_.draft() && !state_.draft()->draftId().isEmpty()) return state_.draft()->draftId();
  return {};
}

QStringList ImportController::draftStackIds() const
{
  QStringList ids;
  const auto rows = runs_->snapshot();
  for (const auto& row : rows) {
    if (!row.draftAttached) continue;
    const QString id = !row.draftId.isEmpty() ? row.draftId : row.logId;
    if (id.isEmpty() || ids.contains(id)) continue;
    ids.push_back(id);
  }
  return ids;
}

int ImportController::activeDraftStackIndex() const
{
  const auto ids = draftStackIds();
  if (ids.isEmpty()) return -1;

  QString activeId = resolveDraftContextLogId();
  if (activeId.isEmpty() && state_.draft() && !state_.draft()->draftId().isEmpty()) {
    activeId = state_.draft()->draftId();
  }

  if (activeId.isEmpty()) return -1;
  return ids.indexOf(activeId);
}

ImportRunRow ImportController::upsertRunById(const QString& logId,
                                             const QString& status,
                                             const QString& message,
                                             bool draftAttached,
                                             const QString& draftId,
                                             const QString& statementId)
{
  ImportRunRow row;
  row.logId = logId.isEmpty() ? generateLogId() : logId;
  const int existingIndex = runs_->findByLogId(row.logId);
  if (existingIndex >= 0) {
    row = runs_->at(existingIndex);
  } else {
    row.type = ui::text::importRuns::typeStatement();
    row.file = state_.currentRunFile();
  }
  row.time = currentTimestamp();
  row.status = status;
  row.message = message;
  row.draftAttached = draftAttached;
  row.draftId = draftId;
  row.statementId = statementId;
  runs_->upsertRun(row);
  return row;
}

ImportRunRow ImportController::upsertActiveDraftRun(const QString& status, const QString& message,
                                                    bool draftAttached, const QString& statementId)
{
  const QString logId = resolveDraftContextLogId();
  const QString draftId = draftAttached ? logId : QString();
  auto row = upsertRunById(logId, status, message, draftAttached, draftId, statementId);

  if (draftAttached) {
    activeDraftLogId_ = row.logId;
  } else if (!activeDraftLogId_.isEmpty() && row.logId == activeDraftLogId_) {
    activeDraftLogId_.clear();
  }
  return row;
}

bool ImportController::ensureJobBridge() {
  if (jobBridge_)
    return true;
  if (!jobSystemFactory_)
    return false;

  auto jobSystem = jobSystemFactory_();
  if (!jobSystem)
    return false;

  jobBridge_ = std::make_unique<importing::ImportJobBridge>(std::move(jobSystem));
  jobBridge_->setExceptionReporter(
      [this](const char *origin, std::exception_ptr exception) {
        reportException(origin, exception);
      });
  return true;
}

void ImportController::addFiles(const QStringList &paths) {
  if (state_.addFiles(paths))
    emit stateChanged();
}

void ImportController::setSelectedFile(const QString &path) {
  if (state_.setSelectedFile(path))
    emit stateChanged();
}

void ImportController::resetStatus() {
  if (state_.resetStatus())
    emit stateChanged();
}

void ImportController::clearDraft() {
  const bool shouldStartNext = state_.clearDraft();
  emit stateChanged();
  if (shouldStartNext)
    startNextQueuedImport();
}

bool ImportController::hasPersistedDraft() const {
  if (!stateSnapshotProvider_)
    return false;
  const auto snapshot = stateSnapshotProvider_();
  return !snapshot.statementDrafts.empty() && !snapshot.transactionDrafts.empty();
}

bool ImportController::openPersistedDraft(const QString& logId) {
  if (!stateSnapshotProvider_)
    return false;

  QString requestedLogId = !logId.isEmpty() ? logId : activeDraftLogId_;
  if (requestedLogId.isEmpty()) {
    const auto rows = runs_->snapshot();
    for (const auto& row : rows) {
      if (!row.draftAttached) continue;
      requestedLogId = !row.draftId.isEmpty() ? row.draftId : row.logId;
      if (!requestedLogId.isEmpty()) break;
    }
  }
  if (!requestedLogId.isEmpty()) {
    activeDraftLogId_ = requestedLogId;
  }

  const auto snapshot = stateSnapshotProvider_();
  const bool restored = restoreDraftFromState(snapshot);
  if (restored) {
    emit stateChanged();
  }
  return restored;
}

bool ImportController::restoreDraftFromState(const core::domain::AppState& snapshot) {
  if (snapshot.statementDrafts.empty()) {
    return false;
  }

  std::shared_ptr<core::domain::StatementDraft> persistedStatementDraft;
  const QString requestedDraftId = resolveDraftContextLogId();
  if (!requestedDraftId.isEmpty()) {
    for (const auto& draft : snapshot.statementDrafts) {
      if (draft && QString::fromStdString(draft->id) == requestedDraftId) {
        persistedStatementDraft = draft;
        break;
      }
    }
  }
  if (!persistedStatementDraft) {
    persistedStatementDraft = snapshot.statementDrafts.front();
  }
  if (!persistedStatementDraft) {
    return false;
  }

  const QString restoredDraftId = QString::fromStdString(persistedStatementDraft->id);
  if (!restoredDraftId.isEmpty()) {
    activeDraftLogId_ = restoredDraftId;
  }

  std::vector<core::domain::TransactionDraft> txDrafts;
  txDrafts.reserve(snapshot.transactionDrafts.size());
  for (const auto& tx : snapshot.transactionDrafts) {
    if (!tx)
      continue;
    if (!persistedStatementDraft->id.empty() &&
        tx->statementDraftId != persistedStatementDraft->id)
      continue;
    txDrafts.push_back(*tx);
  }

  std::sort(txDrafts.begin(), txDrafts.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.position < rhs.position;
  });

  auto statement = std::make_shared<core::domain::Statement>();
  statement->name = persistedStatementDraft->name;

  return state_.restoreDraft(statement,
                             snapshot,
                             txDrafts,
                             restoredDraftId,
                             persistedStatementDraft->currentTransactionIndex,
                             this);
}

void ImportController::rejectImportStart(const QString &errorMessage,
                                         const char *traceMessage) {
  state_.rejectStart(errorMessage);
  observability::reportFlow(
      core::errors::ErrorSeverity::Warning,
      observability::codes::FlowImportRejected,
      observability::origins::controller::importFlow::kStart, traceMessage);
  emit stateChanged();
  emit importFailed(state_.error());
}

void ImportController::requestImportCancellation(bool clearQueue,
                                                 const char *origin,
                                                 const char *traceMessage) {
  if (!state_.isRunning())
    return;

  const auto queuedBeforeCancel = clearQueue ? state_.queuedFiles() : QStringList{};

  state_.beginCancel(clearQueue);

  if (clearQueue && !queuedBeforeCancel.isEmpty()) {
    for (const auto& queuedPath : queuedBeforeCancel) {
      ImportRunRow row;
      row.logId = generateLogId();
      row.time = currentTimestamp();
      row.type = ui::text::importRuns::typeStatement();
      row.file = queuedPath;
      row.status = ui::text::importRuns::statusCanceled();
      row.message = QStringLiteral("Import canceled before start.");
      row.draftAttached = false;
      runs_->upsertRun(row);
    }
    persistRuns();
  }

  if (jobBridge_)
    jobBridge_->cancelCurrent();
  observability::reportFlow(core::errors::ErrorSeverity::Info,
                            observability::codes::FlowImportCanceled, origin,
                            traceMessage,
                            {{observability::context::kFile,
                              strings::toStdString(state_.currentRunFile())},
                             {observability::context::kQueuedCount,
                              std::to_string(state_.queuedFiles().size())}});
  emit stateChanged();
}

void ImportController::handleJobEvent(const core::jobs::JobEvent& event) {
  const double progress = clampProgress(event.progress);
  const QString phase = QString::fromStdString(event.message);
  const auto eventState = event.state;
  const QString message = QString::fromStdString(event.message);

  QMetaObject::invokeMethod(
      this,
      [this, progress, phase, eventState, message]() {
        updateProgress(progress, phase);
        if (eventState == core::jobs::JobState::Finished ||
            eventState == core::jobs::JobState::Failed ||
            eventState == core::jobs::JobState::Canceled) {
          onJobTerminal(eventState, message);
        }
      },
      Qt::QueuedConnection);
}

void ImportController::handleImportCanceled(const QString &now) {
  state_.recordCanceled(now);
  if (!activeRunTerminalHandled_) {
    upsertRunById(activeRunLogId_,
                  ui::text::importRuns::statusCanceled(),
                  ui::text::importPhases::canceled(),
                  false);
    activeRunTerminalHandled_ = true;
  }
  persistRuns();
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled,
      observability::origins::controller::importFlow::kTerminal,
      "Import canceled",
      {{observability::context::kStatus,
        strings::toStdString(ui::text::importRuns::statusCanceled())}});
  emit stateChanged();
  emit importCanceled();
}

void ImportController::handleImportFailed(const QString &now,
                                          const QString &errorMessage,
                                          const char *traceMessage) {
  state_.recordFailed(now, errorMessage);
  if (!activeRunTerminalHandled_) {
    upsertRunById(activeRunLogId_,
                  ui::text::importRuns::statusFailed(),
                  errorMessage,
                  false);
    activeRunTerminalHandled_ = true;
  }
  persistRuns();
  observability::reportFlow(
      core::errors::ErrorSeverity::Warning,
      observability::codes::FlowImportFailed,
      observability::origins::controller::importFlow::kTerminal, traceMessage,
      {{observability::context::kError, strings::toStdString(state_.error())}});
  emit stateChanged();
  emit importFailed(state_.error());
}

bool ImportController::populateDraftFromResult(const QString &now) {
  if (!jobBridge_) {
    handleImportFailed(now, ui::text::controllerErrors::importFailed(),
                       "Import failed: job bridge unavailable");
    return false;
  }

  auto imported = jobBridge_->statementResult();
  if (!imported) {
    handleImportFailed(now, ui::text::controllerErrors::importFailed(),
                       "Import failed: missing statement result");
    return false;
  }

  const auto importedTransactions = jobBridge_->statementTransactions();
  const auto artifacts = jobBridge_->takeArtifacts();
  const auto snapshot = stateSnapshotProvider_ ? stateSnapshotProvider_() : core::domain::AppState{};
  const QString draftId = activeRunDraftId_.isEmpty() ? activeRunLogId_ : activeRunDraftId_;
  const bool hadVisibleDraft = state_.draft() != nullptr;

  if (!saveImportedDraft(draftId, imported, importedTransactions)) {
    handleImportFailed(now, ui::text::controllerErrors::importFailed(),
                       "Import failed: unable to persist draft state");
    return false;
  }

  if (hadVisibleDraft) {
    state_.recordFinished(now);
  } else if (!state_.populateDraft(now,
                                   imported,
                                   snapshot,
                                   importedTransactions,
                                   artifacts,
                                   draftId,
                                   0,
                                   this)) {
      handleImportFailed(now, ui::text::controllerErrors::importFailed(),
                         "Import failed: unable to create statement draft");
      return false;
  }

  upsertRunById(activeRunLogId_,
                ui::text::importRuns::statusDraft(),
                QStringLiteral("Draft ready for manual review."),
                true,
                draftId);
  if (!hadVisibleDraft) {
    activeDraftLogId_ = activeRunLogId_;
  }
  activeRunTerminalHandled_ = true;
  persistRuns();

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportFinished,
      observability::origins::controller::importFlow::kTerminal,
      "Import finished",
      {{observability::context::kStatus,
        strings::toStdString(ui::text::importRuns::statusSuccess())},
       {observability::context::kArtifactCount,
        std::to_string(state_.artifactCount())}});
  emit stateChanged();
  emit importFinished();
  return true;
}

bool ImportController::saveImportedDraft(
    const QString& draftId,
    const std::shared_ptr<core::domain::Statement>& statement,
    const std::vector<core::domain::TransactionDraft>& transactions) const
{
  if (!statementDraftStore_ || !statement || draftId.isEmpty()) {
    return false;
  }

  core::domain::StatementDraft draft;
  draft.id = strings::toStdString(draftId);
  QString name = QString::fromStdString(statement->name);
  if (name.trimmed().isEmpty()) {
    const auto sourceFile = state_.currentRunFile();
    name = QFileInfo(sourceFile).baseName();
  }
  draft.name = strings::toStdString(name);
  draft.currentTransactionIndex = 0;
  draft.transactions = transactions;
  statementDraftStore_(draft);
  return true;
}

void ImportController::cancelImport() {
  requestImportCancellation(
      false, observability::origins::controller::importFlow::kCancel,
      "Import cancellation requested");
}

void ImportController::cancelAllImports() {
  requestImportCancellation(
      true, observability::origins::controller::importFlow::kCancelAll,
      "Cancel-all requested for import queue");
}

void ImportController::startNextQueuedImport() {
  QString next;
  if (!state_.takeNextQueuedFile(next))
    return;
  emit stateChanged();
  startImportForFile(next);
}

void ImportController::startStatementImport() {
  if (state_.isRunning() || state_.draft())
    return;

  const auto t = state_.takeSelectedFileForStart();
  if (!t.isEmpty()) {
    emit stateChanged();
    startImportForFile(t);
    return;
  }

  startNextQueuedImport();
}

void ImportController::startImportForFile(const QString &path) {
  if (state_.isRunning())
    return;
  if (!ensureJobBridge() || !jobBridge_ || !jobBridge_->isAvailable()) {
    rejectImportStart(
        ui::text::controllerErrors::importControllerUnavailable(),
        "Import start rejected: controller unavailable");
    return;
  }
  if (path.trimmed().isEmpty()) {
    rejectImportStart(ui::text::controllerErrors::noFileSelected(),
                      "Import start rejected: no file selected");
    return;
  }

  QString runRootQ;
  const auto spec = buildImportSpec(path, runRootQ);

  state_.beginImport(path);
  activeRunLogId_ = generateLogId();
  activeRunDraftId_ = activeRunLogId_;
  activeRunTerminalHandled_ = false;
  upsertRunById(activeRunLogId_,
                ui::text::importRuns::statusRunning(),
                ui::text::importPhases::starting(),
                false);
  persistRuns();
  emit stateChanged();

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportStarted,
      observability::origins::controller::importFlow::kStart, "Import started",
      {{observability::context::kFile, strings::toStdString(path)},
       {observability::context::kRunRoot, strings::toStdString(runRootQ)},
       {observability::context::kQueuedCount,
        std::to_string(state_.queuedFiles().size())}});

  const bool started = jobBridge_->startStatementImport(
      spec,
      [this](const core::jobs::JobEvent &event) { handleJobEvent(event); });

  if (!started) {
    handleImportFailed(currentTimestamp(),
                       ui::text::controllerErrors::importFailed(),
                       "Import failed: unable to start job");
  }
}

void ImportController::updateProgress(double p, const QString &phase) {
  static const QRegularExpression re(ui::config::kImportProgressPagePattern);
  state_.updateProgress(p, phase, re);
  emit stateChanged();
}

void ImportController::onJobTerminal(core::jobs::JobState state,
                                     const QString& message) {
  if ((!state_.isRunning() && !state_.cancelRequested()) || activeRunTerminalHandled_) {
    return;
  }

  const auto now = currentTimestamp();

  if (state == core::jobs::JobState::Canceled || state_.cancelRequested()) {
    if (jobBridge_)
      jobBridge_->clearSubscription();
    handleImportCanceled(now);
    return;
  }

  if (state == core::jobs::JobState::Failed) {
    if (jobBridge_)
      jobBridge_->clearSubscription();
    handleImportFailed(now,
                       message.isEmpty()
                           ? ui::text::controllerErrors::importFailed()
                           : message,
                       "Import failed");
    return;
  }

  const bool populated = populateDraftFromResult(now);
  if (jobBridge_)
    jobBridge_->clearSubscription();
  if (!populated)
    return;

  startNextQueuedImport();
}

void ImportController::reportException(const char *origin,
                                       std::exception_ptr exception) const {
  if (!errorReporter_)
    return;

  errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin,
                                  exception);
}

} // namespace ui
