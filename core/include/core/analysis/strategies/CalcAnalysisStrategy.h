#pragma once

#include "core/analysis/IAnalysisStrategy.h"

class CalcAnalysisStrategy : public IAnalysisStrategy {
public:
    CalcAnalysisStrategy() = default;
    ~CalcAnalysisStrategy() override = default;

    Analysis compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const override;
};

