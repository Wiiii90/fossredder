#pragma once

#include <string>
#include <vector>

#include "core/models/AnalysisResult.h"

namespace core::domain {
struct AppState;
}

namespace core::analysis {

std::vector<AnalysisTransaction> collectTransactionsForAnalysis(const core::domain::AppState& state,
                                                                const std::string& filterSpec);

}
