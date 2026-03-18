/**
 * @file ui/src/controllers/ExportController.cpp
 * @brief Implements the asynchronous export workflow exposed to the UI.
 */

#include "ui/controllers/ExportController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/text/Text.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <exception>
#include <string>

namespace ui {

ExportController::ExportController(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<ui::exporting::ExportRunner> runner, QObject *parent)
    : QObject(parent), stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      runner_(runner ? std::move(runner)
                     : std::make_shared<ui::exporting::ExportRunner>()) {
  connect(&exportWatcher_,
          &QFutureWatcher<ui::exporting::ExportResult>::finished, this,
          &ExportController::onExportFinished);
}

ui::exporting::ExportRequest
ExportController::buildRequest(ui::controllers::contracts::ExportFormat format,
                               const QString &path, bool includeFormulas,
                               const QString &locale) const {
  ui::exporting::ExportRequest request;
  request.format = format;
  request.path = path;
  request.includeFormulas = includeFormulas;
  request.locale = locale;
  return request;
}

void ExportController::finishExport(bool success) {
  isRunning_ = false;
  if (!success)
    emit exportFailed(lastError_);
  emit exportFinished(success);
}

std::shared_ptr<const AppState> ExportController::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : std::shared_ptr<const AppState>{};
}

void ExportController::exportData(int format, const QString &path,
                                  bool includeFormulas, const QString &locale) {
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
    const auto request = buildRequest(
        static_cast<ui::controllers::contracts::ExportFormat>(format), path,
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
      emit exportFailed(lastError_);
      emit exportFinished(false);
      return;
    }

    isRunning_ = true;

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
          return runner->run(std::move(snapshot), request);
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
    controllers::guard::reportException(
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

void ExportController::onExportFinished() {
  bool success = false;
  try {
    const auto result = exportFuture_.result();
    success = result.success;
    if (!success) {
      lastError_ = result.message.isEmpty()
                       ? ui::text::controllerErrors::exportFailed()
                       : result.message;
      core::errors::report(
          core::errors::ErrorSeverity::Warning,
          result.errorCode.isEmpty() ? core::errors::codes::GenericError
                                     : result.errorCode.toUtf8().constData(),
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
    controllers::guard::reportException(
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
