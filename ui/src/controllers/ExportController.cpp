#include "ui/controllers/ExportController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerContracts.h"
#include "ui/export/ExportRunner.h"
#include "ui/observability/Trace.h"
#include "ui/text/Text.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <exception>
#include <string>

namespace ui {

ExportController::ExportController(AppStateController* core,
                                   std::shared_ptr<ui::exporting::ExportRunner> runner,
                                   QObject* parent)
    : QObject(parent)
    , core_(core)
    , runner_(runner ? std::move(runner) : std::make_shared<ui::exporting::ExportRunner>())
{
    connect(&exportWatcher_, &QFutureWatcher<core::controllers::exporting::ExportOptions>::finished, this, &ExportController::onExportFinished);
}

void ExportController::exportData(int format, const QString& path, bool includeFormulas, const QString& locale)
{
    if (!controllers::guard::ensureCore(core_, "ui::ExportController::exportData")) return;
    if (isRunning_) {
        observability::reportFlow(core::errors::ErrorSeverity::Info,
                                  core::errors::codes::UiFlowExportStarted,
                                  "ui::ExportController::exportData",
                                  "Export ignored: already running");
        return;
    }

    try {
        lastError_.clear();
        isRunning_ = true;
        emit stateChanged();

        ui::exporting::ExportRequest request;
        request.format = format;
        request.path = path;
        request.includeFormulas = includeFormulas;
        request.locale = locale;

        auto opts = runner_->createOptions(core_->state(), request);

        observability::reportFlow(core::errors::ErrorSeverity::Info,
                                  core::errors::codes::UiFlowExportStarted,
                                  "ui::ExportController::exportData",
                                  "Export started",
                                  {
                                      {"path", path.toStdString()},
                                      {"format", std::to_string(format)},
                                      {"includeFormulas", includeFormulas ? "true" : "false"},
                                      {"locale", locale.toStdString()}
                                  });

        exportFuture_ = QtConcurrent::run([runner = runner_, opts = std::move(opts)]() mutable {
            return runner->run(std::move(opts));
        });
        exportWatcher_.setFuture(exportFuture_);
    } catch (const std::exception& ex) {
        core::errors::report(core::errors::ErrorSeverity::Error,
                             core::errors::codes::ExceptionStd,
                             "ui::ExportController::exportData",
                             ex.what());
        lastError_ = tr(ui::text::controllerErrors::kExportFailed);
        observability::reportFlow(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::UiFlowExportFailed,
                                  "ui::ExportController::exportData",
                                  "Export failed with exception",
                                  {
                                      {"exception", ex.what()},
                                      {"path", path.toStdString()}
                                  });
        isRunning_ = false;
        emit stateChanged();
        emit exportFailed(lastError_);
        emit exportFinished(false);
    } catch (...) {
        controllers::guard::reportException("ui::ExportController::exportData");
        lastError_ = tr(ui::text::controllerErrors::kExportFailed);
        observability::reportFlow(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::UiFlowExportFailed,
                                  "ui::ExportController::exportData",
                                  "Export failed with non-std exception",
                                  {
                                      {"path", path.toStdString()}
                                  });
        isRunning_ = false;
        emit stateChanged();
        emit exportFailed(lastError_);
        emit exportFinished(false);
    }
}

void ExportController::onExportFinished()
{
    bool success = false;
    try {
        const auto result = exportFuture_.result();
        success = (result.status == core::controllers::exporting::ExportOptions::Status::Ok);
        if (!success) {
            lastError_ = result.message.empty() ? tr(ui::text::controllerErrors::kExportFailed) : QString::fromStdString(result.message);
            core::errors::report(core::errors::ErrorSeverity::Warning,
                                 result.errorCode.empty() ? core::errors::codes::GenericError : result.errorCode.c_str(),
                                 "ui::ExportController::onExportFinished",
                                 result.message.empty() ? lastError_.toStdString() : result.message);
            observability::reportFlow(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::UiFlowExportFailed,
                                      "ui::ExportController::onExportFinished",
                                      "Export finished with failure",
                                      {
                                          {"error", lastError_.toStdString()}
                                      });
            emit exportFailed(lastError_);
        } else {
            lastError_.clear();
            observability::reportFlow(core::errors::ErrorSeverity::Info,
                                      core::errors::codes::UiFlowExportFinished,
                                      "ui::ExportController::onExportFinished",
                                      "Export finished successfully");
        }
    } catch (const std::exception& ex) {
        core::errors::report(core::errors::ErrorSeverity::Error,
                             core::errors::codes::ExceptionStd,
                             "ui::ExportController::onExportFinished",
                             ex.what());
        lastError_ = tr(ui::text::controllerErrors::kExportFailed);
        observability::reportFlow(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::UiFlowExportFailed,
                                  "ui::ExportController::onExportFinished",
                                  "Export finished with std exception",
                                  {
                                      {"exception", ex.what()}
                                  });
        emit exportFailed(lastError_);
        success = false;
    } catch (...) {
        controllers::guard::reportException("ui::ExportController::onExportFinished");
        lastError_ = tr(ui::text::controllerErrors::kExportFailed);
        observability::reportFlow(core::errors::ErrorSeverity::Error,
                                  core::errors::codes::UiFlowExportFailed,
                                  "ui::ExportController::onExportFinished",
                                  "Export finished with non-std exception");
        emit exportFailed(lastError_);
        success = false;
    }

    isRunning_ = false;
    emit stateChanged();
    emit exportFinished(success);
}

}
