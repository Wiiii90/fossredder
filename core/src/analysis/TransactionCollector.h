/**
 * @file core/src/analysis/TransactionCollector.h
 * @brief Declares private transaction projection helpers for analysis execution.
 */

#pragma once

#include "core/models/AnalysisResult.h"

#include <string>
#include <vector>

namespace core::domain {
struct AppState;
}

namespace core::analysis {

std::vector<AnalysisTransaction> collectTransactionsForAnalysis(const core::domain::AppState& state,
                                                                const std::string& filterSpec);

}
