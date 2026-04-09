/**
 * @file ui/src/controllers/AnalysisController.cpp
 * @brief Implements the UI controller for analysis creation and execution.
 */

#include "ui/controllers/AnalysisController.h"

#include <algorithm>

#include "core/application/AnalysisRequestComposer.h"

#include "core/application/AnalysisService.h"
#include "core/application/AppStateFacade.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "ui/analysis/AnalysisInputMapper.h"
#include "ui/analysis/AnalysisPayloadMapper.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"
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

QString composeFilterSpec(const QString& dateFrom, const QString& dateTo)
{
    return QString::fromStdString(core::application::AnalysisRequestComposer::buildFilterSpec(
        strings::toStdString(dateFrom),
        strings::toStdString(dateTo)));
}

QVariantMap findAnalysisPayload(const std::vector<std::shared_ptr<core::domain::Analysis>>& items,
                               const QString& id)
{
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

} // namespace

AnalysisController::AnalysisController(
    core::application::AppStateFacade* core,
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<core::application::AnalysisService> analysisService,
    QObject* parent)
    : QObject(parent)
    , core_(core)
    , stateSnapshotProvider_(std::move(stateSnapshotProvider))
    , analysisService_(std::move(analysisService))
{
}

QVariantMap AnalysisController::analysis(const QString& id) const
{
    if (!core_) {
        return {};
    }

    return findAnalysisPayload(core_->state().analyses, id);
}

QVariantList AnalysisController::analyses() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().analyses) : QVariantList{};
}

QString AnalysisController::createAnalysis(const QString& name,
                                           const QString& type,
                                           const QString& configJson,
                                           const QString& filterSpec)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::analysis::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addAnalysis(
                strings::toStdString(name),
                strings::toStdString(type),
                strings::toStdString(configJson),
                strings::toStdString(filterSpec)));
        });
}

void AnalysisController::updateAnalysis(const QString& id,
                                        const QString& name,
                                        const QString& type,
                                        const QString& configJson,
                                        const QString& filterSpec)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::analysis::kUpdate, [&]() {
            core_->updateAnalysis(strings::toStdString(id),
                                  strings::toStdString(name),
                                  strings::toStdString(type),
                                  strings::toStdString(configJson),
                                  strings::toStdString(filterSpec));
        });
}

void AnalysisController::deleteAnalysis(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::analysis::kDelete, [&]() {
            core_->deleteAnalysis(strings::toStdString(id));
        });
}

QString AnalysisController::analysisConfigJson(const QString& type,
                                              const QString& plotType,
                                              const QString& plotMeasure,
                                              const QStringList& propertyIds,
                                              const QStringList& contractTypes,
                                              double taxPercent) const
{
    return QString::fromStdString(core::application::AnalysisRequestComposer::buildConfigJson(
        strings::toStdString(type),
        strings::toStdString(plotType),
        strings::toStdString(plotMeasure),
        strings::toStdList(propertyIds),
        strings::toStdList(contractTypes),
        taxPercent));
}

QString AnalysisController::analysisFilterSpec(const QString& dateFrom, const QString& dateTo) const
{
    return composeFilterSpec(dateFrom, dateTo);
}

QString AnalysisController::analysisAdjustmentsJson(const QVariantList& transactions,
                                                    const QVariantList& selectedTransactionIds,
                                                    double taxPercent) const
{
    return QString::fromStdString(core::application::AnalysisRequestComposer::buildTaxAdjustmentsJson(
        ui::analysis::input::toCoreTransactions(transactions),
        ui::analysis::input::toSelectedTransactionIds(selectedTransactionIds),
        taxPercent));
}

QVariantMap AnalysisController::computeAnalysis(const QString& analysisId,
                                                const QString& filterSpec) const
{
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
        if (!result.found) {
            return out;
        }
        return ui::analysis::toPayload(result);
    } catch (...) {
        ui::util::guard::reportException(
            observability::origins::controller::analysis::kCompute);
    }

    return out;
}

QStringList AnalysisController::contractTypes() const
{
    return ui::util::guard::invokeValue<QStringList>(
        core_, observability::origins::controller::analysis::kCompute, {}, [&]() {
            QStringList values;
            for (const auto& type : core_->contractTypes()) {
                values.push_back(QString::fromStdString(type));
            }
            return values;
        });
}

std::shared_ptr<const AppState> AnalysisController::stateSnapshot() const
{
    return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                  : std::shared_ptr<const AppState>{};
}

} // namespace ui
