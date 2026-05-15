/**
 * @file ui/src/export/ExportRunner.cpp
 * @brief Implements guarded export execution against state snapshots.
 */

#include "ui/workflows/export/ExportRunner.h"

#include <string>

#include "ui/shared/config/Defaults.h"
#include "ui/workflows/export/WorkspaceSnapshot.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/observability/Trace.h"
#include "ui/shared/text/Text.h"

namespace ui::exporting {

ExportRunner::ExportRunner(ExecuteExportFn execute)
    : execute_(std::move(execute)) {}

ExportResult ExportRunner::run(std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> state,
                               const ExportRequest &request) const {
  if (!execute_) {
    const QString message = ui::text::exportRunner::runnerUnavailable();
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        ui::observability::codes::FlowExportFailed,
        ui::observability::origins::service::exportRunner::kRun,
        message.toStdString());
    return {
        false,
        core::application::exporting::ExportStatus::InternalError,
        static_cast<core::application::exporting::ExportFormat>(request.format),
        request.path.toStdString(),
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable).toStdString(),
        message.toStdString()};
  }

  if (!state) {
    const QString message = ui::text::exportRunner::stateSnapshotUnavailable();
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        ui::observability::codes::FlowExportFailed,
        ui::observability::origins::service::exportRunner::kRun,
        message.toStdString());
    return {
        false,
        core::application::exporting::ExportStatus::InternalError,
        static_cast<core::application::exporting::ExportFormat>(request.format),
        request.path.toStdString(),
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable).toStdString(),
        message.toStdString()};
  }

  return execute_(std::move(state), request);
}

ExportResult ExportRunner::run(const core::domain::catalog::WorkspaceCatalog &state,
                               const ExportRequest &request) const {
  return run(createWorkspaceSnapshot(state), request);
}

} // namespace ui::exporting

