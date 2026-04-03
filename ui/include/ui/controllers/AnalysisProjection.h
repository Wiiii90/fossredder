/**
 * @file ui/include/ui/controllers/AnalysisProjection.h
 * @brief Declares analysis UI projection helpers shared by controller methods.
 */

#pragma once

#include <QVariantList>
#include <QString>

#include <string>
#include <vector>

#include "core/models/AnalysisResult.h"

namespace ui::analysis::projection {

std::string strategyTypeForIndex(int strategyIndex);
std::vector<core::domain::AnalysisTransaction> toCoreTransactions(const QVariantList& transactions);
std::vector<std::string> toSelectedTransactionIds(const QVariantList& selectedTransactionIds);
double parsePercentOrZero(const QString& value);

} // namespace ui::analysis::projection
