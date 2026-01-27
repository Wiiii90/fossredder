#include "core/analysis/AnalysisController.h"
#include "core/models/AppState.h"
#include "core/models/Analysis.h"
#include "core/analysis/AnalysisResult.h"
#include "core/analysis/strategies/TabAnalysisStrategy.h"
#include "core/analysis/strategies/PlotAnalysisStrategy.h"
#include "core/analysis/Filter.h"

AnalysisController::AnalysisController()
{
    // register built-in strategies
    strategies_.emplace("tab", std::make_unique<TabAnalysisStrategy>());
    strategies_.emplace("plot", std::make_unique<PlotAnalysisStrategy>());
}

AnalysisController::~AnalysisController() = default;

const IAnalysisStrategy* AnalysisController::resolveStrategy(const Analysis& a) const {
    const std::string key = a.type.empty() ? "tab" : a.type;
    auto it = strategies_.find(key);
    if (it != strategies_.end()) return it->second.get();
    // fallback to tab
    auto it2 = strategies_.find("tab");
    return it2 != strategies_.end() ? it2->second.get() : nullptr;
}

AnalysisResult AnalysisController::computeAnalysisById(const std::string& analysisId, const AppState& state, const std::string& filterSpec) const {
    // find analysis in state
    for (const auto& a : state.analyses) {
        if (!a) continue;
        if (a->id == analysisId) return computeAnalysis(*a, state, filterSpec);
    }
    return AnalysisResult{};
}

AnalysisResult AnalysisController::computeAnalysis(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    const IAnalysisStrategy* strat = resolveStrategy(analysis);
    if (!strat) return AnalysisResult{};
    // Always call strategy with full state and pass filterSpec through so strategies
    // can decide how to apply filtering. This guarantees all transactions are
    // available to strategies that need to examine the full dataset.
    return strat->compute(analysis, state, filterSpec);
}
