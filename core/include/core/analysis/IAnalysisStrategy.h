#pragma once

#include <string>
#include "core/models/Analysis.h"
#include "core/models/AppState.h"

class IAnalysisStrategy {
public:
    virtual ~IAnalysisStrategy() = default;

    // compute result for given analysis using the provided AppState and optional filter spec
    virtual Analysis compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const = 0;
};
