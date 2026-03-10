#pragma once

#include <map>
#include <memory>
#include <string>

#include "core/analysis/IAnalysisStrategy.h"

struct AppState;
class Analysis;

// AnalysisEngine resolves analysis strategies and executes analyses against
// an AppState. It is intentionally lightweight and stateless; strategies are
// owned by the engine and can be extended or replaced for testing.
class AnalysisEngine {
public:
    AnalysisEngine();
    ~AnalysisEngine();

    Analysis computeAnalysisById(const std::string& analysisId, const AppState& state, const std::string& filterSpec = "") const;
    Analysis computeAnalysis(const Analysis& analysis, const AppState& state, const std::string& filterSpec = "") const;

private:
    std::map<std::string, std::unique_ptr<IAnalysisStrategy>> strategies_;

    const IAnalysisStrategy* resolveStrategy(const Analysis& analysis) const;
};
