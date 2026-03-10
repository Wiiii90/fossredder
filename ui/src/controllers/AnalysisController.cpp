#include "ui/controllers/AnalysisController.h"

#include <QSet>

#include "core/analysis/AnalysisEngine.h"
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

void reportMissingAnalysisEngine() {
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

QStringList collectContractTypes(const AppState &state) {
  QStringList contractTypes;
  QSet<QString> seen;

  for (const auto &contract : state.contracts) {
    if (!contract)
      continue;
    const QString type = QString::fromStdString(contract->type).trimmed();
    if (type.isEmpty() || seen.contains(type))
      continue;
    seen.insert(type);
    contractTypes.push_back(type);
  }

  contractTypes.sort(Qt::CaseInsensitive);
  return contractTypes;
}

} // namespace

AnalysisController::AnalysisController(
    AppStateController *core, StateSnapshotProvider stateSnapshotProvider,
    const AnalysisEngine *analysisEngine, QObject *parent)
    : QObject(parent), core_(core),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      analysisEngine_(analysisEngine) {}

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
  if (!analysisEngine_) {
    reportMissingAnalysisEngine();
    return out;
  }

  const auto snapshot = stateSnapshot();
  if (!snapshot) {
    reportMissingAnalysisState();
    return out;
  }

  try {
    const auto result = analysisEngine_->computeAnalysisById(
        strings::toStdString(analysisId), *snapshot,
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
  return collectContractTypes(*snapshot);
}

std::shared_ptr<const AppState> AnalysisController::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : std::shared_ptr<const AppState>{};
}

} // namespace ui
