#pragma once

#include "core/analysis/IAnalysisStrategy.h"
#include "core/analysis/AnalysisResult.h"
#include "core/models/AppState.h"

class PlotAnalysisStrategy : public IAnalysisStrategy {
public:
    PlotAnalysisStrategy() = default;
    ~PlotAnalysisStrategy() override = default;

    // supports different plot subtypes via analysis.type / analysis.configJson
    AnalysisResult compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const override;
};
