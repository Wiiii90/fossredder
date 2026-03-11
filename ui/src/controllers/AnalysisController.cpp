#include "ui/controllers/AnalysisController.h"

#include "core/application/AnalysisService.h"
#include "core/controllers/AppStateController.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "ui/analysis/AnalysisPayloadMapper.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/text/Text.h"

namespace ui {

namespace {

void reportMissingAnalysisService() {
  core::errors::report(core::errors::ErrorSeverity::Warning,
                       core::errors::codes::GenericError,
                       observability::origins::controller::analysis::kCompute,
                       ui::text::controllerErrors::kAnalysisEngineUnavailable);
}

void reportMissingAnalysisState() {
  core::errors::report(core::errors::ErrorSeverity::Warning,
                       core::errors::codes::GenericError,
                       observability::origins::controller::analysis::kCompute,
                       ui::text::controllerErrors::kAnalysisStateUnavailable);
}

} // namespace

AnalysisController::AnalysisController(
    core::controllers::AppStateController *core,
    StateSnapshotProvider stateSnapshotProvider,
    const core::application::AnalysisService *analysisService,
    QObject *parent)
    : QObject(parent), core_(core),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      analysisService_(analysisService) {}

QString AnalysisController::addAnalysis(const QString &name,
                                        const QString &type,
                                        const QString &configJson,
                                        const QString &filterSpec) {
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::analysis::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addAnalysis(
            strings::toStdString(name), strings::toStdString(type),
            strings::toStdString(configJson),
            strings::toStdString(filterSpec)));
      });
}

QVariantMap
AnalysisController::computeAnalysis(const QString &analysisId,
                                    const QString &filterSpec) const {
  QVariantMap out;
  if (!analysisService_) {
    reportMissingAnalysisService();
    return out;
  }

  const auto snapshot = stateSnapshot();
  if (!snapshot) {
    reportMissingAnalysisState();
    return out;
  }

  try {
    const auto result = analysisService_->computeAnalysisById(
        *snapshot,
        strings::toStdString(analysisId),
        strings::toStdString(filterSpec));
    if (!result.found)
      return out;
    return ui::analysis::toPayload(result);
  } catch (...) {
    controllers::guard::reportException(
        observability::origins::controller::analysis::kCompute);
  }

  return out;
}

QStringList AnalysisController::getContractTypes() const {
  const auto snapshot = stateSnapshot();
  if (!snapshot)
    return {};

  QStringList values;
  for (const auto &type : analysisService_ ? analysisService_->contractTypes(*snapshot)
                                           : std::vector<std::string>{}) {
    values.push_back(QString::fromStdString(type));
  }
  return values;
}

std::shared_ptr<const AppState> AnalysisController::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : std::shared_ptr<const AppState>{};
}

} // namespace ui
