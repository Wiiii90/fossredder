#pragma once

#include "core/analysis/IAnalysisStrategy.h"
#include "core/models/AppState.h"

class PlotAnalysisStrategy : public IAnalysisStrategy {
public:
    PlotAnalysisStrategy() = default;
    ~PlotAnalysisStrategy() override = default;

    AnalysisResult compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const override;
};
