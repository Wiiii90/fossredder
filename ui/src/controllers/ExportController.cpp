/**
 * @file ui/src/controllers/ExportController.cpp
 * @brief Implements the asynchronous export workflow exposed to the UI.
 */

#include "ui/controllers/ExportController.h"

#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"
#include "ui/text/Text.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QDateTime>
#include <QUuid>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <exception>
#include <string>

namespace ui {

ExportController::ExportController(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::exporting::ExportRunner> runner,
    std::shared_ptr<core::ports::presenters::IExportPresenter> exportPresenter,
    QObject* parent)
    : QObject(parent)
    , stateSnapshotProvider_(std::move(stateSnapshotProvider))
    , runner_(runner ? std::move(runner)
                     : std::make_shared<ui::exporting::ExportRunner>())
    , exportPresenter_(std::move(exportPresenter))
    , runs_(std::make_unique<ExportRunList>(this))
{
    connect(&exportWatcher_,
            &QFutureWatcher<ui::exporting::ExportResult>::finished,
            this,
            &ExportController::onExportFinished);

    restoreRunsFromSnapshot();
}

void ExportController::setExportLogsStore(ExportLogsStore store)
{
    exportLogsStore_ = std::move(store);
    persistRuns();
}

int ExportController::currentMode() const noexcept
{
    if (isRunning_) return ProgressMode;
    return CreateMode;
}

ExportRunList* ExportController::runs() noexcept
{
    return runs_.get();
}

bool ExportController::hasPrevRun() const
{
    return currentRunIndex() > 0;
}

bool ExportController::hasNextRun() const
{
    const int idx = currentRunIndex();
    return idx >= 0 && runs_ && idx < runs_->rowCount() - 1;
}

int ExportController::currentRunIndex() const
{
    if (!runs_ || activeRunLogId_.isEmpty()) return -1;
    return runs_->findByLogId(activeRunLogId_);
}

ui::exporting::ExportRequest ExportController::buildRequest(ui::qml::contracts::ExportFormat format,
                                                            const QString& path,
                                                            bool includeFormulas,
                                                            const QString& locale) const
{
    ui::exporting::ExportRequest request;
    request.format = format;
    request.path = path;
    request.includeFormulas = includeFormulas;
    request.locale = locale;
    request.payload = pendingPayload_;
    return request;
}

void ExportController::finishExport(bool success)
{
    completedSteps_ = totalSteps_;
    progress_ = 1.0;
    phase_ = success ? QStringLiteral("Finished") : QStringLiteral("Failed");

    const QString status = success ? QStringLiteral("Success") : QStringLiteral("Failed");
    upsertRunById(activeRunLogId_, activeRunPath_, status, lastError_, QString());
    persistRuns();

    isRunning_ = false;
    isPaused_ = false;
    cancelRequested_ = false;
    if (!success) {
        emit exportFailed(lastError_);
    }
    emit stateChanged();
    emit exportFinished(success);
}

std::shared_ptr<const core::application::workspace::WorkspaceSessionState> ExportController::stateSnapshot() const
{
    return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                  : std::shared_ptr<const core::application::workspace::WorkspaceSessionState>{};
}

QString ExportController::generateLogId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString ExportController::currentTimestamp() const
{
    return QDateTime::currentDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm:ss"));
}

ExportRunRow ExportController::upsertRunById(const QString& logId,
                                             const QString& path,
                                             const QString& status,
                                             const QString& message,
                                             const QString& payload)
{
    ExportRunRow row;
    row.logId = logId.isEmpty() ? generateLogId() : logId;
    const int existingIndex = runs_ ? runs_->findByLogId(row.logId) : -1;
    if (runs_ && existingIndex >= 0) {
        row = runs_->at(existingIndex);
    }
    row.time = currentTimestamp();
    row.file = path;
    row.status = status;
    row.message = message;
    row.payload = payload;
    if (runs_) runs_->upsertRun(row);
    return row;
}

void ExportController::persistRuns()
{
    if (!exportLogsStore_ || !runs_) return;
    const auto rows = runs_->snapshot();
    std::vector<core::application::exporting::ExportLog> logs;
    logs.reserve(rows.size());
    for (const auto& row : rows) {
        core::application::exporting::ExportLog log;
        log.id = strings::toStdString(row.logId);
        log.time = strings::toStdString(row.time);
        log.targetPath = strings::toStdString(row.file);
        log.status = strings::toStdString(row.status);
        log.message = strings::toStdString(row.message);
        log.payload = strings::toStdString(row.payload);
        logs.push_back(std::move(log));
    }
    exportLogsStore_(logs);
}

void ExportController::restoreRunsFromSnapshot() const
{
    if (!runs_ || !stateSnapshotProvider_) return;
    const auto snapshot = stateSnapshotProvider_();
    if (!snapshot) return;

    std::vector<ExportRunRow> rows;
    const auto& exportLogs = snapshot->workflow.exportLogs;
    rows.reserve(exportLogs.size());
    for (const auto& item : exportLogs) {
        if (!item) continue;
        ExportRunRow row;
        row.logId = QString::fromStdString(item->id);
        row.time = QString::fromStdString(item->time);
        row.file = QString::fromStdString(item->targetPath);
        row.status = QString::fromStdString(item->status);
        row.message = QString::fromStdString(item->message);
        row.payload = QString::fromStdString(item->payload);
        rows.push_back(std::move(row));
    }
    runs_->setRuns(std::move(rows));
}

void ExportController::exportData(int format, const QString &path,
                                  bool includeFormulas, const QString &locale) {
  exportDataWithPayload(format, path, includeFormulas, locale, QString(), 1);
}

void ExportController::exportDataWithPayload(int format,
                                             const QString &path,
                                             bool includeFormulas,
                                             const QString &locale,
                                             const QString& payload,
                                             int totalSteps) {
  if (isRunning_) {
    observability::reportFlow(
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::controller::exportFlow::kStart,
        "Export ignored: already running");
    return;
  }

  try {
    lastError_.clear();
    totalSteps_ = std::max(1, totalSteps);
    completedSteps_ = 0;
    progress_ = 0.0;
    phase_ = QStringLiteral("Starting export...");
    pendingPayload_ = payload;
    activeRunPath_ = path;
    activeRunLogId_ = generateLogId();
    upsertRunById(activeRunLogId_, activeRunPath_, QStringLiteral("Running"), QString(), QString());
    emit stateChanged();

    const auto request = buildRequest(
        static_cast<ui::qml::contracts::ExportFormat>(format), path,
        includeFormulas, locale);
    const auto snapshot = stateSnapshot();
    if (!snapshot) {
      lastError_ = ui::text::controllerErrors::exportStateUnavailable();
      observability::reportFlow(
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::controller::exportFlow::kStart,
          "Export rejected: state snapshot unavailable",
          {{observability::context::kPath, strings::toStdString(path)}});
      upsertRunById(activeRunLogId_, activeRunPath_, QStringLiteral("Failed"), lastError_, QString());
      persistRuns();
      emit exportFailed(lastError_);
      emit exportFinished(false);
      emit stateChanged();
      return;
    }

    isRunning_ = true;
    phase_ = QStringLiteral("Running export...");
    emit stateChanged();

    observability::reportFlow(
        core::errors::ErrorSeverity::Info,
        observability::codes::FlowExportStarted,
        observability::origins::controller::exportFlow::kStart,
        "Export started",
        {{observability::context::kPath, strings::toStdString(path)},
         {observability::context::kFormat,
          std::to_string(static_cast<int>(request.format))},
         {observability::context::kIncludeFormulas,
          includeFormulas ? "true" : "false"},
         {observability::context::kLocale, strings::toStdString(locale)}});

    exportFuture_ = QtConcurrent::run(
        [runner = runner_, snapshot, request = std::move(request)]() mutable {
          return runner->run(snapshot->catalog, request);
        });
    exportWatcher_.setFuture(exportFuture_);
  } catch (const std::exception &ex) {
    core::errors::report(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionStd,
        observability::origins::controller::exportFlow::kStart, ex.what());
    lastError_ = ui::text::controllerErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::controller::exportFlow::kStart,
        "Export failed with exception",
        {{observability::context::kException, ex.what()},
         {observability::context::kPath, strings::toStdString(path)}});
    finishExport(false);
  } catch (...) {
    ui::util::guard::reportException(
        observability::origins::controller::exportFlow::kStart);
    lastError_ = ui::text::controllerErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::controller::exportFlow::kStart,
        "Export failed with non-std exception",
        {{observability::context::kPath, strings::toStdString(path)}});
    finishExport(false);
  }
}

void ExportController::activateRunAt(int index)
{
  if (!runs_) return;
  const auto row = runs_->at(index);
  if (row.logId.isEmpty()) return;
  activeRunLogId_ = row.logId;
  emit runActivated(row.payload);
  emit stateChanged();
}

bool ExportController::openRunLocationAt(int index)
{
  if (!runs_) return false;
  const auto row = runs_->at(index);
  if (row.file.isEmpty()) return false;

  QFileInfo info(row.file);
  QString folderPath;

  if (info.exists()) {
    folderPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
  } else {
    const QString asDir = row.file;
    const QDir dir(asDir);
    if (dir.exists()) folderPath = dir.absolutePath();
  }

  if (folderPath.isEmpty()) return false;
  return QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

void ExportController::removeRunAt(int index)
{
  if (!runs_) return;
  const auto row = runs_->at(index);
  if (row.logId == activeRunLogId_) activeRunLogId_.clear();
  runs_->removeAt(index);
  persistRuns();
  emit stateChanged();
}

void ExportController::clearRuns()
{
  if (!runs_) return;
  runs_->clear();
  activeRunLogId_.clear();
  persistRuns();
  emit stateChanged();
}

bool ExportController::openPrevRun()
{
  const int idx = currentRunIndex();
  if (idx <= 0) return false;
  activateRunAt(idx - 1);
  return true;
}

bool ExportController::openNextRun()
{
  const int idx = currentRunIndex();
  if (idx < 0 || !runs_ || idx >= runs_->rowCount() - 1) return false;
  activateRunAt(idx + 1);
  return true;
}

void ExportController::clearActiveRun()
{
  activeRunLogId_.clear();
  emit stateChanged();
}

void ExportController::cancelExport()
{
  if (!isRunning_) return;
  cancelRequested_ = true;
  phase_ = QStringLiteral("Cancel requested...");
  emit stateChanged();
}

void ExportController::togglePause()
{
  if (!isRunning_) return;
  isPaused_ = !isPaused_;
  phase_ = isPaused_ ? QStringLiteral("Paused") : QStringLiteral("Running export...");
  emit stateChanged();
}

void ExportController::onExportFinished() {
  if (cancelRequested_) {
    lastError_.clear();
    phase_ = QStringLiteral("Canceled");
    finishExport(false);
    return;
  }

  bool success = false;
  try {
    const auto result = exportFuture_.result();
    const auto presented = exportPresenter_ ? exportPresenter_->present(result) : result;
    success = presented.success;
    if (!success) {
      lastError_ = presented.message.empty()
                       ? ui::text::controllerErrors::exportFailed()
                       : QString::fromStdString(presented.message);
      core::errors::report(
          core::errors::ErrorSeverity::Warning,
          presented.errorCode.empty() ? core::errors::codes::GenericError
                                      : presented.errorCode.c_str(),
          observability::origins::controller::exportFlow::kFinish,
          strings::toStdString(lastError_));
      observability::reportFlow(
          core::errors::ErrorSeverity::Warning,
          observability::codes::FlowExportFailed,
          observability::origins::controller::exportFlow::kFinish,
          "Export finished with failure",
          {{observability::context::kError, strings::toStdString(lastError_)}});
    } else {
      lastError_.clear();
      observability::reportFlow(
          core::errors::ErrorSeverity::Info,
          observability::codes::FlowExportFinished,
          observability::origins::controller::exportFlow::kFinish,
          "Export finished successfully");
    }
  } catch (const std::exception &ex) {
    core::errors::report(
        core::errors::ErrorSeverity::Error, core::errors::codes::ExceptionStd,
        observability::origins::controller::exportFlow::kFinish, ex.what());
    lastError_ = ui::text::controllerErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::controller::exportFlow::kFinish,
        "Export finished with std exception",
        {{observability::context::kException, ex.what()}});
    success = false;
  } catch (...) {
    ui::util::guard::reportException(
        observability::origins::controller::exportFlow::kFinish);
    lastError_ = ui::text::controllerErrors::exportFailed();
    observability::reportFlow(
        core::errors::ErrorSeverity::Error,
        observability::codes::FlowExportFailed,
        observability::origins::controller::exportFlow::kFinish,
        "Export finished with non-std exception");
    success = false;
  }

  finishExport(success);
}

} // namespace ui
