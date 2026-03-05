#pragma once

#include <memory>
#include <map>
#include <string>

#include "core/analysis/IAnalysisStrategy.h"

class AppState;
class Analysis;

// AnalysisController resolves analysis strategies and executes analyses against
// an AppState. It is intentionally lightweight and stateless; strategies are
// owned by the controller and can be extended or replaced for testing.
class AnalysisController {
public:
    AnalysisController();
    ~AnalysisController();

    // Compute result for analysis identified by id (searches in provided state)
    Analysis computeAnalysisById(const std::string& analysisId, const AppState& state, const std::string& filterSpec = "") const;

    // Compute result for given analysis object
    Analysis computeAnalysis(const Analysis& analysis, const AppState& state, const std::string& filterSpec = "") const;

private:
    // map from strategy key (analysis.type) to strategy implementation
    std::map<std::string, std::unique_ptr<IAnalysisStrategy>> strategies_;

    const IAnalysisStrategy* resolveStrategy(const Analysis& a) const;
};
