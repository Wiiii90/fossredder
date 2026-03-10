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
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::UiFlowExportFailed,
        ui::observability::origins::service::exportRunner::kRun,
        ui::text::exportRunner::kRunnerUnavailable);
    return {
        false,
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable),
        QString::fromLatin1(ui::text::exportRunner::kRunnerUnavailable)};
  }

  if (!state) {
    ui::observability::reportFlow(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::UiFlowExportFailed,
        ui::observability::origins::service::exportRunner::kRun,
        ui::text::exportRunner::kStateSnapshotUnavailable);
    return {
        false,
        QString::fromLatin1(ui::config::errorCodes::kExportRunnerUnavailable),
        QString::fromLatin1(ui::text::exportRunner::kStateSnapshotUnavailable)};
  }

  return execute_(std::move(state), request);
}

ExportResult ExportRunner::run(const AppState &state,
                               const ExportRequest &request) const {
  return run(createSnapshot(state), request);
}

} // namespace ui::exporting
