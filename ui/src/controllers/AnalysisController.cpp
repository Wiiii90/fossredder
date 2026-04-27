/**
 * @file ui/src/controllers/AnalysisController.cpp
 * @brief Implements the UI controller for analysis creation and execution.
 */

#include "ui/controllers/AnalysisController.h"

#include <algorithm>

#include <QSet>

#include "core/application/AnalysisRequestComposer.h"

#include "core/application/AnalysisService.h"
#include "core/application/AppStateFacade.h"
#include "core/analysis/Filter.h"
#include "core/errors/ErrorCodes.h"
#include "core/models/AppState.h"
#include "core/models/Actor.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
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
                                           const QString& filterSpec,
                                           const QString& exportFormat,
                                           bool includeCalcAdjustments,
                                           const QString& exportStateJson,
                                           const QString& snapshotTransactionsJson)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::analysis::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addAnalysis(
                strings::toStdString(name),
                strings::toStdString(type),
                strings::toStdString(configJson),
                strings::toStdString(filterSpec),
                strings::toStdString(exportFormat),
                includeCalcAdjustments,
                strings::toStdString(exportStateJson),
                strings::toStdString(snapshotTransactionsJson)));
        });
}

void AnalysisController::updateAnalysis(const QString& id,
                                        const QString& name,
                                        const QString& type,
                                        const QString& configJson,
                                        const QString& filterSpec,
                                        const QString& exportFormat,
                                        bool includeCalcAdjustments,
                                        const QString& exportStateJson,
                                        const QString& snapshotTransactionsJson)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::analysis::kUpdate, [&]() {
            core_->updateAnalysis(strings::toStdString(id),
                                  strings::toStdString(name),
                                  strings::toStdString(type),
                                  strings::toStdString(configJson),
                                  strings::toStdString(filterSpec),
                                  strings::toStdString(exportFormat),
                                  includeCalcAdjustments,
                                  strings::toStdString(exportStateJson),
                                  strings::toStdString(snapshotTransactionsJson));
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

QString AnalysisController::analysisFilterSpec(const QString& dateMode,
                                               const QString& year,
                                               const QString& dateFrom,
                                               const QString& dateTo,
                                               const QStringList& propertyIds,
                                               const QStringList& contractTypes,
                                               const QString& allocatableMode) const
{
    return QString::fromStdString(core::application::AnalysisRequestComposer::buildFilterSpec(
        strings::toStdString(dateMode),
        strings::toStdString(year),
        strings::toStdString(dateFrom),
        strings::toStdString(dateTo),
        strings::toStdList(propertyIds),
        strings::toStdList(contractTypes),
        strings::toStdString(allocatableMode)));
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

QVariantMap AnalysisController::previewTransactions(const QString& filterSpec) const
{
    QVariantMap out;
    const auto snapshot = stateSnapshot();
    if (!snapshot) {
        return out;
    }

    QHash<QString, QString> contractTypeById;
    QHash<QString, QString> contractNameById;
    contractTypeById.reserve(static_cast<int>(snapshot->contracts.size()));
    contractNameById.reserve(static_cast<int>(snapshot->contracts.size()));
    for (const auto& contract : snapshot->contracts) {
        if (!contract) continue;
        contractTypeById.insert(QString::fromStdString(contract->id), QString::fromStdString(contract->type));
        contractNameById.insert(QString::fromStdString(contract->id), QString::fromStdString(contract->name));
    }

    QHash<QString, QString> actorNameById;
    actorNameById.reserve(static_cast<int>(snapshot->actors.size()));
    for (const auto& actor : snapshot->actors) {
        if (!actor) continue;
        actorNameById.insert(QString::fromStdString(actor->id), QString::fromStdString(actor->name));
    }

    QHash<QString, QString> statementNameById;
    statementNameById.reserve(static_cast<int>(snapshot->statements.size()));
    for (const auto& statement : snapshot->statements) {
        if (!statement) continue;
        statementNameById.insert(QString::fromStdString(statement->id), QString::fromStdString(statement->name));
    }

    QHash<QString, QString> propertyNameById;
    propertyNameById.reserve(static_cast<int>(snapshot->properties.size()));
    for (const auto& property : snapshot->properties) {
        if (!property) continue;
        propertyNameById.insert(QString::fromStdString(property->id), QString::fromStdString(property->name));
    }

    QVariantList transactions;
    QVariantMap metrics;
    QSet<QString> statementIds;
    double amountSum = 0.0;

    const auto parsedFilter = core::analysis::parseFilterSpec(strings::toStdString(filterSpec));
    transactions.reserve(static_cast<int>(snapshot->transactions.size()));

    for (const auto& transaction : snapshot->transactions) {
        if (!transaction) continue;
        if (!filterSpec.isEmpty() && !parsedFilter.matches(transaction, *snapshot)) continue;

        QVariantMap row;
        const QString txId = QString::fromStdString(transaction->id);
        const QString statementId = QString::fromStdString(transaction->statementId);
        const QString contractId = QString::fromStdString(transaction->contractId);
        const QString actorId = QString::fromStdString(transaction->actorId);

        QStringList propertyNames;
        propertyNames.reserve(static_cast<int>(transaction->propertyIds.size()));
        for (const auto& propertyId : transaction->propertyIds) {
            const QString id = QString::fromStdString(propertyId);
            const QString name = propertyNameById.value(id, id);
            if (!name.isEmpty()) propertyNames.push_back(name);
        }

        row[QStringLiteral("id")] = txId;
        row[QStringLiteral("name")] = QString::fromStdString(transaction->name);
        row[QStringLiteral("transactionName")] = QString::fromStdString(transaction->name);
        row[QStringLiteral("date")] = QString::fromStdString(transaction->bookingDate);
        row[QStringLiteral("valuta")] = QString::fromStdString(transaction->valuta);
        row[QStringLiteral("amount")] = transaction->amount;
        row[QStringLiteral("statementId")] = statementId;
        row[QStringLiteral("statementName")] = statementNameById.value(statementId, QString());
        row[QStringLiteral("actorName")] = actorNameById.value(actorId, QString());
        row[QStringLiteral("contractId")] = contractId;
        row[QStringLiteral("contractName")] = contractNameById.value(contractId, QString());
        row[QStringLiteral("contractType")] = contractTypeById.value(contractId, QString());
        row[QStringLiteral("propertyNames")] = propertyNames;
        row[QStringLiteral("propertiesLabel")] = propertyNames.join(QStringLiteral(", "));
        row[QStringLiteral("allocatable")] = transaction->allocatable;

        transactions.push_back(row);
        statementIds.insert(statementId);
        amountSum += transaction->amount;
    }

    metrics[QStringLiteral("statementCount")] = statementIds.size();
    metrics[QStringLiteral("transactionCount")] = transactions.size();
    metrics[QStringLiteral("amountSum")] = amountSum;

    out[QStringLiteral("transactions")] = transactions;
    out[QStringLiteral("metrics")] = metrics;
    return out;
}

std::shared_ptr<const AppState> AnalysisController::stateSnapshot() const
{
    return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                  : std::shared_ptr<const AppState>{};
}

} // namespace ui
