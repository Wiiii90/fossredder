/**
 * @file ui/src/controllers/ImportController.cpp
 * @brief Implements the import workflow controller used by the QML UI.
 */

#include "ui/controllers/ImportController.h"

#include <QDateTime>
#include <QDir>
#include <QMetaObject>
#include <QRegularExpression>

#include <exception>
#include <stdexcept>
#include <string>

#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "ui/config/Defaults.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/import/ImportDraftMapper.h"
#include "ui/import/ImportRunStore.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/text/Text.h"

namespace ui {

namespace {

/** @brief Returns an ISO timestamp for import run bookkeeping. */
QString currentTimestamp() {
  return QDateTime::currentDateTime().toString(Qt::ISODate);
}

/** @brief Clamps import progress values to the configured UI range. */
double clampProgress(double progress) {
  if (progress < ui::config::importProgress::kMinimum)
    return ui::config::importProgress::kMinimum;
  if (progress > ui::config::importProgress::kMaximum)
    return ui::config::importProgress::kMaximum;
  return progress;
}

/** @brief Builds the import job specification and prepares a fresh import run folder. */
core::jobs::ImportStatementJobSpec buildImportSpec(const QString &path,
                                                   QString &runRoot) {
  importing::cleanupOldImportRuns(ui::config::kImportRunKeepCount);

  const auto runInfo = importing::createImportRunInfo();
  runRoot = runInfo.runRoot;

  core::jobs::ImportStatementJobSpec spec;
  spec.sourcePath = strings::toEncodedPath(path);
  spec.runRoot = strings::toEncodedPath(runInfo.runRoot);
  spec.proofOutputDir = strings::toEncodedPath(QDir(runInfo.runRoot).filePath("proof"));

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
    : QObject(parent), runs_(std::make_unique<ImportRunList>(this)), state_(*runs_),
      jobSystemFactory_(std::move(jobSystemFactory)),
      errorReporter_(std::move(errorReporter)) {
  if (!errorReporter_)
    throw std::invalid_argument("ImportController requires an error reporter");
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

ImportRunList *ImportController::runs() noexcept {
  return runs_.get();
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

  state_.beginCancel(clearQueue);
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

void ImportController::handleJobEvent(const core::jobs::JobEvent &event) {
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
  if (!state_.populateDraft(now, imported, importedTransactions, artifacts, this)) {
    handleImportFailed(now, ui::text::controllerErrors::importFailed(),
                       "Import failed: unable to create statement draft");
    return false;
  }

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

  state_.beginImport(path);
  emit stateChanged();

  QString runRootQ;
  const auto spec = buildImportSpec(path, runRootQ);

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
                                     const QString &message) {
  const auto now = currentTimestamp();
  if (jobBridge_)
    jobBridge_->clearSubscription();

  if (state == core::jobs::JobState::Canceled || state_.cancelRequested()) {
    handleImportCanceled(now);
    return;
  }

  if (state == core::jobs::JobState::Failed) {
    handleImportFailed(now,
                       message.isEmpty()
                           ? ui::text::controllerErrors::importFailed()
                           : message,
                       "Import failed");
    return;
  }

  populateDraftFromResult(now);
}

void ImportController::reportException(const char *origin,
                                       std::exception_ptr exception) const {
  if (!errorReporter_)
    return;

  errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin,
                                  exception);
}

} // namespace ui
