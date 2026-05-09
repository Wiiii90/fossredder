/**
 * @file core/src/analysis/strategies/PlotAnalysisStrategy.h
 * @brief Declares the private plot analysis strategy implementation.
 */

#pragma once

#include "../AnalysisStrategy.h"

namespace core::analysis {

class PlotAnalysisStrategy : public AnalysisStrategy {
public:
    PlotAnalysisStrategy() = default;
    ~PlotAnalysisStrategy() override = default;

    AnalysisResult compute(const Analysis& analysis, const AppState& state, const Filter& filter) const override;
};

}
