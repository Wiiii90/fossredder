/**
 * @file ui/include/ui/analysis/AnalysisInputMapper.h
 * @brief Declares helpers that map analysis-related UI input into core-friendly values.
 */

#pragma once

#include <QString>
#include <QVariantList>

#include <string>
#include <vector>

#include "core/application/analysis/RunAnalysisResult.h"

namespace ui::analysis::input {

/** @brief Convert a QML transaction list into core analysis transactions.
 *  @param transactions List of serialized transactions
 *  @return Vector of core AnalysisTransaction
 */
std::vector<core::domain::AnalysisTransaction> toCoreTransactions(const QVariantList& transactions);

/** @brief Convert selected QML transaction identifiers into trimmed core identifiers.
 *  @param selectedTransactionIds List of selected transaction identifiers
 *  @return Vector of trimmed std::string identifiers
 */
std::vector<std::string> toSelectedTransactionIds(const QVariantList& selectedTransactionIds);

} // namespace ui::analysis::input
