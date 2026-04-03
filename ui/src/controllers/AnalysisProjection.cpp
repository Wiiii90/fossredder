/**
 * @file ui/src/controllers/AnalysisProjection.cpp
 * @brief Implements analysis UI projection helpers shared by controller methods.
 */

#include "ui/controllers/AnalysisProjection.h"

#include <cmath>

namespace ui::analysis::projection {

std::string strategyTypeForIndex(int strategyIndex)
{
    switch (strategyIndex) {
    case 0:
        return "tab";
    case 2:
        return "calc";
    case 1:
    default:
        return "plot";
    }
}

std::vector<core::domain::AnalysisTransaction> toCoreTransactions(const QVariantList& transactions)
{
    std::vector<core::domain::AnalysisTransaction> out;
    out.reserve(transactions.size());
    for (const auto& item : transactions) {
        const auto row = item.toMap();
        if (row.isEmpty()) continue;

        core::domain::AnalysisTransaction transaction;
        transaction.id = row.value(QStringLiteral("id")).toString().toStdString();
        transaction.amount = row.value(QStringLiteral("amount")).toDouble();
        out.push_back(std::move(transaction));
    }
    return out;
}

std::vector<std::string> toSelectedTransactionIds(const QVariantList& selectedTransactionIds)
{
    std::vector<std::string> out;
    out.reserve(selectedTransactionIds.size());
    for (const auto& item : selectedTransactionIds) {
        const auto value = item.toString().trimmed();
        if (!value.isEmpty()) out.push_back(value.toStdString());
    }
    return out;
}

double parsePercentOrZero(const QString& value)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok && std::isfinite(parsed) ? parsed : 0.0;
}

} // namespace ui::analysis::projection
