/**
 * @file ui/src/controllers/AnalysisController.cpp
 * @brief Implements the UI controller for analysis creation and execution.
 */

#include "ui/controllers/AnalysisController.h"

#include <QVariantMap>

#include "core/application/AnalysisService.h"
#include "core/application/AppStateFacade.h"
#include "core/application/AnalysisRequestComposer.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "core/models/AnalysisResult.h"
#include "ui/analysis/AnalysisPayloadMapper.h"
#include "ui/controllers/AnalysisProjection.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/text/Text.h"

namespace ui {

namespace {

/** @brief Reports that no analysis service was configured for the controller. */
void reportMissingAnalysisService()
{
  core::errors::report(core::errors::ErrorSeverity::Warning,
                       core::errors::codes::GenericError,
                       observability::origins::controller::analysis::kCompute,
                       ui::text::controllerErrors::analysisEngineUnavailable().toStdString());
}

/** @brief Reports that no application state snapshot is currently available. */
void reportMissingAnalysisState()
{
  core::errors::report(core::errors::ErrorSeverity::Warning,
                       core::errors::codes::GenericError,
                       observability::origins::controller::analysis::kCompute,
                       ui::text::controllerErrors::analysisStateUnavailable().toStdString());
}

} // namespace

AnalysisController::AnalysisController(
    core::application::AppStateFacade *core,
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<core::application::AnalysisService> analysisService,
    QObject *parent)
    : QObject(parent), core_(core),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      analysisService_(std::move(analysisService)) {}

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

QString AnalysisController::createAnalysisFromUi(const QString& name,
                                                 const QString& type,
                                                 const QString& plotType,
                                                 const QString& plotMeasure,
                                                 const QStringList& propertyIds,
                                                 const QStringList& contractTypes,
                                                 const QString& dateFrom,
                                                 const QString& dateTo,
                                                 double taxPercent)
{
  const auto configJson = QString::fromStdString(core::application::AnalysisRequestComposer::buildConfigJson(
      strings::toStdString(type),
      strings::toStdString(plotType),
      strings::toStdString(plotMeasure),
      strings::toStdList(propertyIds),
      strings::toStdList(contractTypes),
      taxPercent));
  const auto filterSpec = QString::fromStdString(core::application::AnalysisRequestComposer::buildFilterSpec(
      strings::toStdString(dateFrom),
      strings::toStdString(dateTo)));
  return addAnalysis(name, type, configJson, filterSpec);
}

QVariantMap AnalysisController::createAnalysisFromUiAndCompute(const QString& name,
                                                               const QString& type,
                                                               const QString& plotType,
                                                               const QString& plotMeasure,
                                                               const QStringList& propertyIds,
                                                               const QStringList& contractTypes,
                                                               const QString& dateFrom,
                                                               const QString& dateTo,
                                                               double taxPercent)
{
  QVariantMap out;
  const auto id = createAnalysisFromUi(name, type, plotType, plotMeasure, propertyIds, contractTypes, dateFrom, dateTo, taxPercent);
  if (id.isEmpty()) return out;

  out.insert(QStringLiteral("id"), id);
  const auto filterSpec = buildFilterSpec(dateFrom, dateTo);
  const auto result = computeAnalysis(id, filterSpec);
  if (!result.isEmpty()) out.insert(QStringLiteral("analysisResult"), result);
  return out;
}

QVariantMap AnalysisController::createAnalysisFromStrategyAndCompute(const QString& name,
                                                                     int strategyIndex,
                                                                     const QString& plotType,
                                                                     const QString& plotMeasure,
                                                                     const QStringList& propertyIds,
                                                                     const QStringList& contractTypes,
                                                                     const QString& dateFrom,
                                                                     const QString& dateTo,
                                                                     double taxPercent)
{
  return createAnalysisFromUiAndCompute(name,
                                        QString::fromStdString(ui::analysis::projection::strategyTypeForIndex(strategyIndex)),
                                        plotType,
                                        plotMeasure,
                                        propertyIds,
                                        contractTypes,
                                        dateFrom,
                                        dateTo,
                                        taxPercent);
}

QString AnalysisController::buildFilterSpec(const QString& dateFrom, const QString& dateTo) const
{
  return QString::fromStdString(core::application::AnalysisRequestComposer::buildFilterSpec(
      strings::toStdString(dateFrom),
      strings::toStdString(dateTo)));
}

QString AnalysisController::buildTaxAdjustmentsJson(const QVariantList& transactions,
                                                    const QVariantList& selectedTransactionIds,
                                                    double taxPercent) const
{
  return QString::fromStdString(core::application::AnalysisRequestComposer::buildTaxAdjustmentsJson(
      ui::analysis::projection::toCoreTransactions(transactions),
      ui::analysis::projection::toSelectedTransactionIds(selectedTransactionIds),
      taxPercent));
}

QVariantMap AnalysisController::applyTaxAdjustmentsAndRecompute(const QString& analysisId,
                                                                const QString& filterSpec,
                                                                const QVariantList& transactions,
                                                                const QVariantList& selectedTransactionIds,
                                                                double taxPercent) const
{
  QVariantMap out;
  const auto adjustmentsJson = buildTaxAdjustmentsJson(transactions, selectedTransactionIds, taxPercent);
  out.insert(QStringLiteral("adjustmentsJson"), adjustmentsJson);

  const auto result = computeAnalysis(analysisId, filterSpec);
  if (!result.isEmpty()) out.insert(QStringLiteral("analysisResult"), result);
  return out;
}

QVariantMap AnalysisController::applyTaxAdjustmentsAndRecomputeFromText(const QString& analysisId,
                                                                        const QString& filterSpec,
                                                                        const QVariantList& transactions,
                                                                        const QVariantList& selectedTransactionIds,
                                                                        const QString& taxPercentText) const
{
  return applyTaxAdjustmentsAndRecompute(analysisId,
                                         filterSpec,
                                         transactions,
                                         selectedTransactionIds,
                                         ui::analysis::projection::parsePercentOrZero(taxPercentText));
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
  return controllers::guard::invokeValue<QStringList>(
      core_, observability::origins::controller::analysis::kCompute, {},
      [&]() {
        QStringList values;
        for (const auto &type : core_->contractTypes()) {
          values.push_back(QString::fromStdString(type));
        }
        return values;
      });
}

std::shared_ptr<const AppState> AnalysisController::stateSnapshot() const {
  return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                : std::shared_ptr<const AppState>{};
}

} // namespace ui
