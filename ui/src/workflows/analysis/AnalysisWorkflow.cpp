/**
 * @file ui/src/workflows/analysis/AnalysisWorkflow.cpp
 * @brief Implements the UI workflow for analysis creation and execution.
 */

#include "ui/workflows/analysis/AnalysisWorkflow.h"

#include <algorithm>

#include <QSet>
#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QStandardPaths>

#include "core/application/analysis/AnalysisService.h"
#include "core/application/analysis/AnalysisFilterSpec.h"
#include "core/errors/ErrorCodes.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/ports/analysis-image-renderer/IAnalysisImageRenderer.h"
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

std::vector<std::pair<std::string, double>> parseAdjustments(const QString& adjustmentsJson)
{
    std::vector<std::pair<std::string, double>> out;
    const QJsonDocument document = QJsonDocument::fromJson(adjustmentsJson.toUtf8());
    if (!document.isObject()) {
        return out;
    }
    const QJsonObject object = document.object();
    out.reserve(static_cast<std::size_t>(object.size()));
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (!it.value().isDouble()) {
            continue;
        }
        out.emplace_back(it.key().toStdString(), it.value().toDouble());
    }
    return out;
}

} // namespace

AnalysisWorkflow::AnalysisWorkflow(
    core::ports::workspace::IWorkspaceWriter* core,
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<core::application::analysis::AnalysisService> analysisService,
    std::shared_ptr<core::ports::presenters::IAnalysisPresenter> analysisPresenter,
    QObject* parent,
    std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
    , stateSnapshotProvider_(std::move(stateSnapshotProvider))
    , analysisService_(std::move(analysisService))
    , analysisPresenter_(std::move(analysisPresenter))
    , imageRenderer_(std::move(imageRenderer))
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
                                           const QString& snapshotTransactionsJson,
                                           const QString& adjustmentsJson)
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
            command.adjustments = parseAdjustments(adjustmentsJson);
            return QString::fromStdString(core_->addAnalysis(command));
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

QString AnalysisWorkflow::analysisFilterSpec(const QString& dateField,
                                             const QString& dateMode,
                                             const QString& year,
                                             const QString& dateFrom,
                                             const QString& dateTo,
                                             const QStringList& propertyIds,
                                             const QStringList& contractTypes,
                                             const QString& allocatableMode) const
{
    core::application::analysis::AnalysisFilterSelection selection;
    selection.dateField = dateField.trimmed().toLower().toStdString();
    selection.dateMode = dateMode.trimmed().toLower().toStdString();
    selection.year = year.trimmed().toStdString();
    selection.dateFrom = dateFrom.trimmed().toStdString();
    selection.dateTo = dateTo.trimmed().toStdString();
    selection.allocatableMode = allocatableMode.trimmed().toLower().toStdString();
    for (const auto& propertyId : propertyIds) {
        const QString value = propertyId.trimmed();
        if (!value.isEmpty()) {
            selection.propertyIds.push_back(value.toStdString());
        }
    }
    for (const auto& contractType : contractTypes) {
        const QString value = contractType.trimmed().toLower();
        if (!value.isEmpty()) {
            selection.contractTypes.push_back(value.toStdString());
        }
    }
    return QString::fromStdString(core::application::analysis::buildAnalysisFilterSpec(selection));
}

QVariantMap AnalysisWorkflow::parseAnalysisFilterSpec(const QString& filterSpec) const
{
    const auto parsed = core::application::analysis::parseAnalysisFilterSelection(filterSpec.toStdString());
    QVariantMap out;
    out.insert(QStringLiteral("dateField"), QString::fromStdString(parsed.dateField));
    out.insert(QStringLiteral("dateMode"), QString::fromStdString(parsed.dateMode));
    out.insert(QStringLiteral("year"), QString::fromStdString(parsed.year));
    out.insert(QStringLiteral("dateFrom"), QString::fromStdString(parsed.dateFrom));
    out.insert(QStringLiteral("dateTo"), QString::fromStdString(parsed.dateTo));

    QVariantList propertyIds;
    propertyIds.reserve(static_cast<int>(parsed.propertyIds.size()));
    for (const auto& id : parsed.propertyIds) {
        propertyIds.push_back(QString::fromStdString(id));
    }
    out.insert(QStringLiteral("propertyIds"), propertyIds);
    out.insert(QStringLiteral("propertyIdsNone"), parsed.propertyIdsUnassigned);

    QVariantList contractTypes;
    contractTypes.reserve(static_cast<int>(parsed.contractTypes.size()));
    for (const auto& type : parsed.contractTypes) {
        contractTypes.push_back(QString::fromStdString(type));
    }
    out.insert(QStringLiteral("contractTypes"), contractTypes);
    out.insert(QStringLiteral("contractTypesNone"), parsed.contractTypesUnassigned);
    out.insert(QStringLiteral("allocatableMode"), QString::fromStdString(parsed.allocatableMode));
    return out;
}

QString AnalysisWorkflow::analysisConfigJson(const QString& type,
                                             const QString& plotType,
                                             const QString& plotMeasure,
                                             const QStringList& propertyIds,
                                             const QStringList& contractTypes,
                                             double taxPercent) const
{
    QJsonObject config;
    const QString normalizedType = type.trimmed().toLower();

    if (normalizedType == QStringLiteral("calc")) {
        config.insert(QStringLiteral("strategy"), QStringLiteral("tax"));
        config.insert(QStringLiteral("percent"), taxPercent);
        return QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact));
    }

    config.insert(QStringLiteral("plotType"), plotType.trimmed().isEmpty() ? QStringLiteral("pie") : plotType.trimmed());
    config.insert(QStringLiteral("plotMeasure"), plotMeasure.trimmed().isEmpty() ? QStringLiteral("totalAmount") : plotMeasure.trimmed());

    QJsonArray properties;
    for (const auto& propertyId : propertyIds) {
        const QString value = propertyId.trimmed();
        if (!value.isEmpty()) {
            properties.push_back(value);
        }
    }
    config.insert(QStringLiteral("properties"), properties);

    QJsonArray types;
    for (const auto& contractType : contractTypes) {
        const QString value = contractType.trimmed();
        if (!value.isEmpty()) {
            types.push_back(value);
        }
    }
    config.insert(QStringLiteral("contractTypes"), types);

    return QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact));
}

QString AnalysisWorkflow::analysisAdjustmentsJson(const QVariantList& transactions,
                                                  const QStringList& selectedTransactionIds,
                                                  double taxPercent) const
{
    QSet<QString> selected;
    selected.reserve(selectedTransactionIds.size());
    for (const auto& id : selectedTransactionIds) {
        const QString value = id.trimmed();
        if (!value.isEmpty()) {
            selected.insert(value);
        }
    }

    QJsonObject adjustments;
    if (selected.isEmpty()) {
        return QStringLiteral("{}");
    }

    const double factor = 1.0 + (taxPercent / 100.0);
    for (const auto& value : transactions) {
        const QVariantMap row = value.toMap();
        const QString id = row.value(QStringLiteral("id")).toString().trimmed();
        if (id.isEmpty() || !selected.contains(id)) {
            continue;
        }
        const double amount = row.value(QStringLiteral("amount")).toDouble();
        adjustments.insert(id, amount * factor);
    }

    return QString::fromUtf8(QJsonDocument(adjustments).toJson(QJsonDocument::Compact));
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
        auto result = analysisService_->runAnalysis(*snapshot, request);
        if (!result.found) {
            return out;
        }
        return payloadWithRenderedPreview(request, std::move(result));
    } catch (...) {
        ui::util::guard::reportException(
            observability::origins::workflow::analysis::kCompute);
    }

    return out;
}

QVariantMap AnalysisWorkflow::computeAnalysis(const QString& analysisId,
                                              const QString& filterSpecification) const
{
    return computeAnalysis(analysisRequest(analysisId, filterSpecification));
}

QVariantMap AnalysisWorkflow::computeAnalysisPreview(const QString& analysisId,
                                                     const QString& filterSpecification,
                                                     bool includeCalcAdjustments,
                                                     const QString& adjustmentsJson) const
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

    const std::string targetId = analysisId.trimmed().toStdString();
    const auto it = std::find_if(snapshot->analyses().begin(), snapshot->analyses().end(), [&](const auto& analysis) {
        return analysis && analysis->id() == targetId;
    });
    if (it == snapshot->analyses().end() || !*it) {
        return out;
    }

    try {
        const auto& source = **it;
        core::domain::Analysis preview;
        preview.setId(source.id());
        preview.rename(source.name());
        preview.setType(source.type());
        preview.setConfigJson(source.configJson());
        preview.setFilterSpec(source.filterSpec());
        preview.setExportFormat(source.exportFormat());
        preview.setExportStateJson(source.exportStateJson());
        preview.setSnapshotTransactionsJson(source.snapshotTransactionsJson());
        preview.setIncludeCalculationAdjustments(includeCalcAdjustments);
        for (const auto& [key, value] : source.adjustments()) {
            preview.setAdjustment(key, value);
        }
        for (const auto& [key, value] : parseAdjustments(adjustmentsJson)) {
            preview.setAdjustment(key, value);
        }

        const auto request = analysisRequest(analysisId, filterSpecification);
        auto result = analysisService_->computeAnalysis(preview, *snapshot, request.filterSpecification);
        if (!result.found) {
            return out;
        }
        return payloadWithRenderedPreview(request, std::move(result));
    } catch (...) {
        ui::util::guard::reportException(
            observability::origins::workflow::analysis::kCompute);
    }

    return out;
}

QVariantMap AnalysisWorkflow::payloadWithRenderedPreview(const core::application::analysis::AnalysisRequest& request,
                                                         core::application::analysis::AnalysisResult result) const
{
    if (imageRenderer_) {
        const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                + QStringLiteral("/fossredder-analysis-preview");
        QDir().mkpath(baseDir);
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(QByteArray::fromStdString(request.analysisId));
        hash.addData(QByteArray::fromStdString(request.filterSpecification));
        hash.addData(QByteArray::fromStdString(result.type));
        hash.addData(QByteArray::fromStdString(result.configJson));
        for (const auto& row : result.table) {
            for (const auto& column : row) {
                hash.addData(QByteArray::fromStdString(column));
                hash.addData(QByteArray(1, '\0'));
            }
            hash.addData(QByteArray(1, '\n'));
        }
        const QString fileName = QString::fromLatin1(hash.result().toHex()) + QStringLiteral(".png");
        const QString outputPath = QDir(baseDir).filePath(fileName);
        if (QFileInfo::exists(outputPath)
                || imageRenderer_->writeAnalysisImage(outputPath.toStdString(), request.analysisId, result)) {
            result.artifacts.push_back(outputPath.toStdString());
        }
    }
    return ::ui::analysis::toPayload(result);
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
    QHash<QString, QStringList> contractPropertyIdsById;
    contractTypeById.reserve(static_cast<int>(snapshot->contracts().size()));
    contractNameById.reserve(static_cast<int>(snapshot->contracts().size()));
    contractPropertyIdsById.reserve(static_cast<int>(snapshot->contracts().size()));
    for (const auto& contract : snapshot->contracts()) {
        if (!contract) continue;
        const QString id = QString::fromStdString(contract->id());
        QStringList propertyIds;
        propertyIds.reserve(static_cast<int>(contract->propertyIds().size()));
        for (const auto& propertyId : contract->propertyIds()) {
            propertyIds.push_back(QString::fromStdString(propertyId));
        }
        contractTypeById.insert(id, QString::fromStdString(contract->type()));
        contractNameById.insert(id, QString::fromStdString(contract->name()));
        contractPropertyIdsById.insert(id, propertyIds);
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

        QStringList propertyIds;
        propertyIds.reserve(static_cast<int>(transaction->propertyIds().size()));
        for (const auto& propertyId : transaction->propertyIds()) {
            const QString id = QString::fromStdString(propertyId);
            if (!id.isEmpty() && !propertyIds.contains(id)) {
                propertyIds.push_back(id);
            }
        }
        for (const auto& propertyId : contractPropertyIdsById.value(contractId)) {
            if (!propertyId.isEmpty() && !propertyIds.contains(propertyId)) {
                propertyIds.push_back(propertyId);
            }
        }

        QStringList propertyNames;
        propertyNames.reserve(propertyIds.size());
        for (const auto& id : propertyIds) {
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
        row[QStringLiteral("propertyIds")] = propertyIds;
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
