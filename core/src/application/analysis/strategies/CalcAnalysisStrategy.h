/**
 * @file core/src/analysis/strategies/CalcAnalysisStrategy.h
 * @brief Declares the private calculation analysis strategy implementation.
 */

#pragma once

#include "../AnalysisStrategy.h"

namespace core::analysis {

class CalcAnalysisStrategy : public AnalysisStrategy {
public:
    CalcAnalysisStrategy() = default;
    ~CalcAnalysisStrategy() override = default;

    AnalysisResult compute(const Analysis& analysis, const AppState& state, const Filter& filter) const override;
};

}
