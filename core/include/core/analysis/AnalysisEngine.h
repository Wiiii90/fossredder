#pragma once

#include <map>
#include <memory>
#include <string>

#include "core/analysis/IAnalysisStrategy.h"
#include "core/models/AnalysisResult.h"

namespace core::domain {
struct AppState;
class Analysis;
}

// AnalysisEngine resolves analysis strategies and executes analyses against
// an AppState. It is intentionally lightweight and stateless; strategies are
// owned by the engine and can be extended or replaced for testing.
namespace core::analysis {

class AnalysisEngine {
public:
    AnalysisEngine();
    ~AnalysisEngine();

    AnalysisResult computeAnalysisById(const std::string& analysisId, const core::domain::AppState& state, const std::string& filterSpec = "") const;
    AnalysisResult computeAnalysis(const core::domain::Analysis& analysis, const core::domain::AppState& state, const std::string& filterSpec = "") const;

private:
    std::map<std::string, std::unique_ptr<IAnalysisStrategy>> strategies_;

    const IAnalysisStrategy* resolveStrategy(const core::domain::Analysis& analysis) const;
};

}
