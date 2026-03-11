#include "core/analysis/AnalysisEngine.h"

#include "core/analysis/OutputTypeResolver.h"
#include "core/constants/CoreDefaults.h"
#include "core/analysis/TransactionCollector.h"
#include "core/analysis/strategies/CalcAnalysisStrategy.h"
#include "core/analysis/strategies/PlotAnalysisStrategy.h"
#include "core/analysis/strategies/TabAnalysisStrategy.h"
#include "core/models/Analysis.h"
#include "core/models/AnalysisResult.h"
#include "core/models/AppState.h"

namespace core::analysis {

AnalysisEngine::AnalysisEngine()
{
    strategies_.emplace(std::string(core::constants::analysis::kTypeTab), std::make_unique<TabAnalysisStrategy>());
    strategies_.emplace(std::string(core::constants::analysis::kTypePlot), std::make_unique<PlotAnalysisStrategy>());
    strategies_.emplace(std::string(core::constants::analysis::kTypeCalc), std::make_unique<CalcAnalysisStrategy>());
}

AnalysisEngine::~AnalysisEngine() = default;

const IAnalysisStrategy* AnalysisEngine::resolveStrategy(const Analysis& analysis) const
{
    const std::string key = analysis.type.empty() ? std::string(core::constants::analysis::kTypeTab) : analysis.type;
    const auto it = strategies_.find(key);
    if (it != strategies_.end()) return it->second.get();

    const auto fallback = strategies_.find(std::string(core::constants::analysis::kTypeTab));
    return fallback != strategies_.end() ? fallback->second.get() : nullptr;
}

AnalysisResult AnalysisEngine::computeAnalysisById(const std::string& analysisId,
                                                   const AppState& state,
                                                   const std::string& filterSpec) const
{
    for (const auto& analysis : state.analyses) {
        if (!analysis) continue;
        if (analysis->id != analysisId) continue;
        const std::string effectiveFilter = filterSpec.empty() ? analysis->filterSpec : filterSpec;
        return computeAnalysis(*analysis, state, effectiveFilter);
    }

    return AnalysisResult{};
}

AnalysisResult AnalysisEngine::computeAnalysis(const Analysis& analysis,
                                               const AppState& state,
                                               const std::string& filterSpec) const
{
    const IAnalysisStrategy* strategy = resolveStrategy(analysis);
    AnalysisResult out;
    if (!strategy) return out;

    out = strategy->compute(analysis, state, filterSpec);
    out.type = resolveAnalysisOutputType(analysis);
    out.configJson = analysis.configJson;
    out.transactions = collectTransactionsForAnalysis(state, filterSpec);
    out.generatedAt = analysis.updatedAt.empty() ? analysis.createdAt : analysis.updatedAt;
    out.found = true;
    return out;
}

}
