/**
 * @file core/src/analysis/TransactionCollector.h
 * @brief Declares private transaction projection helpers for analysis execution.
 */

#pragma once

#include "core/application/analysis/Filter.h"
#include "core/application/analysis/RunAnalysisResult.h"

#include <string>
#include <vector>

namespace core::domain {
struct WorkspaceState;
}

namespace core::analysis {

std::vector<AnalysisTransaction> collectTransactionsForAnalysis(const core::domain::WorkspaceState& state,
                                                                 const Filter& filter);

}
