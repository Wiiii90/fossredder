/**
 * @file ui/src/export/ExportRunner.cpp
 * @brief Implements guarded export execution against state snapshots.
 */

#include "ui/export/ExportRunner.h"

#include <string>

#include "ui/config/Defaults.h"
#include "ui/export/AppStateSnapshot.h"
#include "ui/observability/Origins.h"
#include "ui/observability/Trace.h"
#include "ui/text/Text.h"

namespace ui::exporting {

ExportRunner::ExportRunner(ExecuteExportFn execute)
    : execute_(std::move(execute)) {}

ExportResult ExportRunner::run(std::shared_ptr<const AppState> state,
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
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable),
        message};
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
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable),
        message};
  }

  return execute_(std::move(state), request);
}

ExportResult ExportRunner::run(const AppState &state,
                               const ExportRequest &request) const {
  return run(createSnapshot(state), request);
}

} // namespace ui::exporting
