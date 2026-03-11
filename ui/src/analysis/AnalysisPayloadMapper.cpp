#include "ui/analysis/AnalysisPayloadMapper.h"

#include <QCoreApplication>
#include <QVariantList>

#include "core/models/AnalysisResult.h"
#include "ui/payload/PayloadKeys.h"
#include "ui/text/Text.h"

namespace ui::analysis {

namespace {

QVariantMap toMetricsMap(const AnalysisResult& result)
{
    QVariantMap metrics;
    for (const auto& [key, value] : result.metrics) {
        metrics.insert(QString::fromStdString(key), value);
    }
    return metrics;
}

QVariantList toTableList(const AnalysisResult& result)
{
    QVariantList table;
    for (const auto& row : result.table) {
        QVariantList columns;
        for (const auto& column : row) {
            columns.push_back(QString::fromStdString(column));
        }
        table.push_back(columns);
    }
    return table;
}

QVariantList toArtifactList(const AnalysisResult& result)
{
    QVariantList artifacts;
    for (const auto& artifact : result.artifacts) {
        artifacts.push_back(QString::fromStdString(artifact));
    }
    return artifacts;
}

QVariantList toTransactionList(const AnalysisResult& result)
{
    QVariantList transactions;
    for (const auto& transaction : result.transactions) {
        QVariantMap item;
        item[ui::payload::keys::common::kId] = QString::fromStdString(transaction.id);
        item[ui::payload::keys::common::kName] = QString::fromStdString(transaction.name);
        item[ui::payload::keys::transaction::kDate] = QString::fromStdString(transaction.bookingDate);
        item[ui::payload::keys::common::kAmount] = transaction.amount;
        item[ui::payload::keys::transaction::kContractId] = QString::fromStdString(transaction.contractId);
        item[ui::payload::keys::transaction::kContractType] = transaction.contractType.empty()
            ? QCoreApplication::translate(ui::text::contexts::kAnalysisPayloadMapper, ui::text::analysis::kUnassignedContractType)
            : QString::fromStdString(transaction.contractType);
        transactions.push_back(item);
    }
    return transactions;
}

}

QVariantMap toPayload(const AnalysisResult& result)
{
    QVariantMap payload;
    payload[ui::payload::keys::analysis::kMetrics] = toMetricsMap(result);
    payload[ui::payload::keys::analysis::kTable] = toTableList(result);
    payload[ui::payload::keys::common::kType] = QString::fromStdString(result.type);
    payload[ui::payload::keys::analysis::kConfig] = QString::fromStdString(result.configJson);
    payload[ui::payload::keys::analysis::kTransactions] = toTransactionList(result);
    payload[ui::payload::keys::analysis::kArtifacts] = toArtifactList(result);
    payload[ui::payload::keys::analysis::kGeneratedAt] = QString::fromStdString(result.generatedAt);
    return payload;
}

}
