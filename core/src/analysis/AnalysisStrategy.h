/**
 * @file core/src/analysis/AnalysisStrategy.h
 * @brief Declares the private strategy interface used inside the analysis engine implementation.
 */

#pragma once

#include "core/models/Analysis.h"
#include "core/models/AnalysisResult.h"
#include "core/models/AppState.h"

#include <string>

namespace core::analysis {

class AnalysisStrategy {
public:
    virtual ~AnalysisStrategy() = default;
    virtual AnalysisResult compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const = 0;
};

}
