/**
 * @file ui/src/workflows/analysis/AnalysisWorkflow.cpp
 * @brief Implements the UI workflow for analysis creation and execution.
 */

#include "ui/workflows/analysis/AnalysisWorkflow.h"

#include <algorithm>

#include <QSet>

#include "core/application/analysis/AnalysisService.h"
#include "core/errors/ErrorCodes.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/ports/presenters/IAnalysisPresenter.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/adapters/core/AnalysisResultMapper.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/util/CoreFacadeGuard.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/shared/text/Text.h"

namespace ui {

namespace {

/** @brief Reports that no analysis service was configured for the workflow. */
void reportMissingAnalysisService()
{
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::GenericError,
                         observability::origins::workflow::analysis::kCompute,
                         ui::text::workflowErrors::analysisEngineUnavailable().toStdString());
}

/** @brief Reports that no application state snapshot is currently available. */
void reportMissingAnalysisState()
{
    core::errors::report(core::errors::ErrorSeverity::Warning,
                         core::errors::codes::GenericError,
                         observability::origins::workflow::analysis::kCompute,
                         ui::text::workflowErrors::analysisStateUnavailable().toStdString());
}

QVariantMap findAnalysisPayload(const std::vector<core::ports::workspace::AnalysisSnapshot>& items,
                                const QString& id)
{
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (it == items.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(it->id);
    payload[QStringLiteral("name")] = QString::fromStdString(it->name);
    payload[QStringLiteral("type")] = QString::fromStdString(it->type);
    payload[QStringLiteral("configJson")] = QString::fromStdString(it->configJson);
    payload[QStringLiteral("filterSpec")] = QString::fromStdString(it->filterSpec);
    payload[QStringLiteral("exportFormat")] = QString::fromStdString(it->exportFormat);
    payload[QStringLiteral("includeCalculationAdjustments")] = it->includeCalculationAdjustments;
    payload[QStringLiteral("exportStateJson")] = QString::fromStdString(it->exportStateJson);
    payload[QStringLiteral("snapshotTransactionsJson")] = QString::fromStdString(it->snapshotTransactionsJson);
    payload[QStringLiteral("createdAt")] = QString::fromStdString(it->createdAt);
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(it->updatedAt);
    return payload;
}

} // namespace

AnalysisWorkflow::AnalysisWorkflow(
    core::ports::workspace::IWorkspaceWriter* core,
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<core::application::analysis::AnalysisService> analysisService,
    std::shared_ptr<core::ports::presenters::IAnalysisPresenter> analysisPresenter,
    QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
    , stateSnapshotProvider_(std::move(stateSnapshotProvider))
    , analysisService_(std::move(analysisService))
    , analysisPresenter_(std::move(analysisPresenter))
{
}

QVariantMap AnalysisWorkflow::analysis(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    return findAnalysisPayload(reader_->workspaceSnapshot().analyses, id);
}

QVariantList AnalysisWorkflow::analyses() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().analyses;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
        payload[QStringLiteral("type")] = QString::fromStdString(item.type);
        payload[QStringLiteral("configJson")] = QString::fromStdString(item.configJson);
        payload[QStringLiteral("filterSpec")] = QString::fromStdString(item.filterSpec);
        payload[QStringLiteral("exportFormat")] = QString::fromStdString(item.exportFormat);
        payload[QStringLiteral("includeCalculationAdjustments")] = item.includeCalculationAdjustments;
        payload[QStringLiteral("exportStateJson")] = QString::fromStdString(item.exportStateJson);
        payload[QStringLiteral("snapshotTransactionsJson")] = QString::fromStdString(item.snapshotTransactionsJson);
        payload[QStringLiteral("createdAt")] = QString::fromStdString(item.createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(item.updatedAt);
        out.push_back(std::move(payload));
    }
    return out;
}

QString AnalysisWorkflow::createAnalysis(const QString& name,
                                           const QString& type,
                                           const QString& configJson,
                                           const QString& filterSpec,
                                           const QString& exportFormat,
                                           bool includeCalcAdjustments,
                                           const QString& exportStateJson,
                                           const QString& snapshotTransactionsJson)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::workflow::analysis::kAdd, {}, [&]() {
            core::ports::workspace::AnalysisCommand command;
            command.name = strings::toStdString(name);
            command.type = strings::toStdString(type);
            command.configJson = strings::toStdString(configJson);
            command.filterSpec = strings::toStdString(filterSpec);
            command.exportFormat = strings::toStdString(exportFormat);
            command.includeCalculationAdjustments = includeCalcAdjustments;
            command.exportStateJson = strings::toStdString(exportStateJson);
            command.snapshotTransactionsJson = strings::toStdString(snapshotTransactionsJson);
            return QString::fromStdString(core_->addAnalysis(command));
        });
}

void AnalysisWorkflow::updateAnalysis(const QString& id,
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
        core_, observability::origins::workflow::analysis::kUpdate, [&]() {
            core::ports::workspace::AnalysisCommand command;
            command.id = strings::toStdString(id);
            command.name = strings::toStdString(name);
            command.type = strings::toStdString(type);
            command.configJson = strings::toStdString(configJson);
            command.filterSpec = strings::toStdString(filterSpec);
            command.exportFormat = strings::toStdString(exportFormat);
            command.includeCalculationAdjustments = includeCalcAdjustments;
            command.exportStateJson = strings::toStdString(exportStateJson);
            command.snapshotTransactionsJson = strings::toStdString(snapshotTransactionsJson);
            core_->updateAnalysis(command);
        });
}

void AnalysisWorkflow::deleteAnalysis(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::workflow::analysis::kDelete, [&]() {
            core_->deleteAnalysis(strings::toStdString(id));
        });
}

core::application::analysis::AnalysisRequest AnalysisWorkflow::analysisRequest(const QString& analysisId,
                                                                                const QString& filterSpecification) const
{
    core::application::analysis::AnalysisRequest request;
    request.analysisId = analysisId.trimmed().toStdString();
    request.filterSpecification = filterSpecification.trimmed().toStdString();
    return request;
}

QVariantMap AnalysisWorkflow::computeAnalysis(const core::application::analysis::AnalysisRequest& request) const
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
        const auto result = analysisService_->runAnalysis(*snapshot, request);
        if (!result.found) {
            return out;
        }
        return ::ui::analysis::toPayload(result);
    } catch (...) {
        ui::util::guard::reportException(
            observability::origins::workflow::analysis::kCompute);
    }

    return out;
}

QStringList AnalysisWorkflow::contractTypes() const
{
    if (!reader_) {
        return {};
    }
    return ui::util::guard::invokeValue<QStringList>(
        reader_, observability::origins::workflow::analysis::kCompute, {}, [&]() {
            std::vector<std::string> valuesStd;
            QSet<QString> seen;
            for (const auto& contract : reader_->workspaceSnapshot().contracts) {
                if (contract.type.empty()) {
                    continue;
                }
                const auto type = QString::fromStdString(contract.type);
                if (seen.contains(type)) {
                    continue;
                }
                seen.insert(type);
                valuesStd.push_back(contract.type);
            }
            std::sort(valuesStd.begin(), valuesStd.end());
            QStringList values;
            for (const auto& type : valuesStd) {
                values.push_back(QString::fromStdString(type));
            }
            return values;
        });
}

QVariantMap AnalysisWorkflow::previewTransactions(const QString& filterSpecification) const
{
    QVariantMap out;
    const auto snapshot = stateSnapshot();
    if (!snapshot) {
        return out;
    }

    QHash<QString, QString> contractTypeById;
    QHash<QString, QString> contractNameById;
    contractTypeById.reserve(static_cast<int>(snapshot->contracts().size()));
    contractNameById.reserve(static_cast<int>(snapshot->contracts().size()));
    for (const auto& contract : snapshot->contracts()) {
        if (!contract) continue;
        contractTypeById.insert(QString::fromStdString(contract->id()), QString::fromStdString(contract->type()));
        contractNameById.insert(QString::fromStdString(contract->id()), QString::fromStdString(contract->name()));
    }

    QHash<QString, QString> actorNameById;
    actorNameById.reserve(static_cast<int>(snapshot->actors().size()));
    for (const auto& actor : snapshot->actors()) {
        if (!actor) continue;
        actorNameById.insert(QString::fromStdString(actor->id()), QString::fromStdString(actor->name()));
    }

    QHash<QString, QString> statementNameById;
    statementNameById.reserve(static_cast<int>(snapshot->statements().size()));
    for (const auto& statement : snapshot->statements()) {
        if (!statement) continue;
        statementNameById.insert(QString::fromStdString(statement->id()), QString::fromStdString(statement->name()));
    }

    QHash<QString, QString> propertyNameById;
    propertyNameById.reserve(static_cast<int>(snapshot->properties().size()));
    for (const auto& property : snapshot->properties()) {
        if (!property) continue;
        propertyNameById.insert(QString::fromStdString(property->id()), QString::fromStdString(property->name()));
    }

    QVariantList transactions;
    QVariantMap metrics;
    QSet<QString> statementIds;
    double amountSum = 0.0;

    const auto filtered = analysisService_->filterTransactions(*snapshot, strings::toStdString(filterSpecification));
    transactions.reserve(static_cast<int>(filtered.size()));

    for (const auto& transaction : filtered) {
        if (!transaction) continue;

        QVariantMap row;
        const QString txId = QString::fromStdString(transaction->id());
        const QString statementId = QString::fromStdString(transaction->statementId());
        const QString contractId = QString::fromStdString(transaction->contractId());
        const QString actorId = QString::fromStdString(transaction->actorId());

        QStringList propertyNames;
        propertyNames.reserve(static_cast<int>(transaction->propertyIds().size()));
        for (const auto& propertyId : transaction->propertyIds()) {
            const QString id = QString::fromStdString(propertyId);
            const QString name = propertyNameById.value(id, id);
            if (!name.isEmpty()) propertyNames.push_back(name);
        }

        row[QStringLiteral("id")] = txId;
        row[QStringLiteral("name")] = QString::fromStdString(transaction->name());
        row[QStringLiteral("transactionName")] = QString::fromStdString(transaction->name());
        row[QStringLiteral("date")] = QString::fromStdString(transaction->bookingDate());
        row[QStringLiteral("valuta")] = QString::fromStdString(transaction->valuta());
        row[QStringLiteral("amount")] = transaction->amount();
        row[QStringLiteral("statementId")] = statementId;
        row[QStringLiteral("statementName")] = statementNameById.value(statementId, QString());
        row[QStringLiteral("actorName")] = actorNameById.value(actorId, QString());
        row[QStringLiteral("contractId")] = contractId;
        row[QStringLiteral("contractName")] = contractNameById.value(contractId, QString());
        row[QStringLiteral("contractType")] = contractTypeById.value(contractId, QString());
        row[QStringLiteral("propertyNames")] = propertyNames;
        row[QStringLiteral("propertiesLabel")] = propertyNames.join(QStringLiteral(", "));
        row[QStringLiteral("allocatable")] = transaction->isAllocatable();

        transactions.push_back(row);
        statementIds.insert(statementId);
        amountSum += transaction->amount();
    }

    metrics[QStringLiteral("statementCount")] = statementIds.size();
    metrics[QStringLiteral("transactionCount")] = transactions.size();
    metrics[QStringLiteral("amountSum")] = amountSum;

    out[QStringLiteral("transactions")] = transactions;
    out[QStringLiteral("metrics")] = metrics;
    return out;
}

std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> AnalysisWorkflow::stateSnapshot() const
{
    return stateSnapshotProvider_ ? stateSnapshotProvider_()
                                  : std::shared_ptr<const core::domain::catalog::WorkspaceCatalog>{};
}

} // namespace ui

