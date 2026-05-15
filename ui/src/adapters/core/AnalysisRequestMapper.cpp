/**
 * @file ui/src/analysis/AnalysisInputMapper.cpp
 * @brief Implements helpers that map analysis-related UI input into core-friendly values.
 */

#include "ui/adapters/core/AnalysisRequestMapper.h"

#include <utility>

namespace ui::analysis::input {

std::vector<core::domain::AnalysisTransaction> toCoreTransactions(const QVariantList& transactions)
{
    std::vector<core::domain::AnalysisTransaction> out;
    out.reserve(transactions.size());
    for (const auto& item : transactions) {
        const auto row = item.toMap();
        if (row.isEmpty()) {
            continue;
        }

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
        if (!value.isEmpty()) {
            out.push_back(value.toStdString());
        }
    }
    return out;
}

} // namespace ui::analysis::input
