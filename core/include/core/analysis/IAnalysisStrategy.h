#pragma once

#include <string>
#include "core/models/Analysis.h"
#include "core/analysis/AnalysisResult.h"
#include "core/models/AppState.h"

class AnalysisResult;

class IAnalysisStrategy {
public:
    virtual ~IAnalysisStrategy() = default;

    // compute result for given analysis using the provided AppState and optional filter spec
    virtual AnalysisResult compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const = 0;
};
