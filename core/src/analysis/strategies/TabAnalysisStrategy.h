/**
 * @file core/src/analysis/strategies/TabAnalysisStrategy.h
 * @brief Declares the private tabular analysis strategy implementation.
 */

#pragma once

#include "../AnalysisStrategy.h"

namespace core::analysis {

class TabAnalysisStrategy : public AnalysisStrategy {
public:
    TabAnalysisStrategy() = default;
    ~TabAnalysisStrategy() override = default;

    AnalysisResult compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const override;
};

}
