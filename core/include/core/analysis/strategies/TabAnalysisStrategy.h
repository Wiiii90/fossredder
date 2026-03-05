#pragma once

#include "core/analysis/IAnalysisStrategy.h"
#include "core/models/AppState.h"

class TabAnalysisStrategy : public IAnalysisStrategy {
public:
    TabAnalysisStrategy() = default;
    ~TabAnalysisStrategy() override = default;

    Analysis compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const override;
};
