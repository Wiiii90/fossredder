/**
 * @file core/src/analysis/AnalysisStrategy.h
 * @brief Declares the private strategy interface used inside the analysis engine implementation.
 */

#pragma once

#include "core/application/analysis/Filter.h"
#include "core/domain/entities/Analysis.h"
#include "core/application/analysis/RunAnalysisResult.h"
#include "core/application/workspace/AppState.h"

#include <string>

namespace core::analysis {

class AnalysisStrategy {
public:
    virtual ~AnalysisStrategy() = default;
    virtual AnalysisResult compute(const Analysis& analysis, const AppState& state, const Filter& filter) const = 0;
};

}
