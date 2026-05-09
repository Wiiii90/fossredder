#include "core/application/analysis/AnalysisEngine.h"

#include "core/application/analysis/Filter.h"
#include "OutputTypeResolver.h"
#include "core/constants/analysis.h"
#include "TransactionCollector.h"
#include "strategies/CalcAnalysisStrategy.h"
#include "strategies/PlotAnalysisStrategy.h"
#include "strategies/TabAnalysisStrategy.h"
#include "core/domain/entities/Analysis.h"
#include "core/application/analysis/RunAnalysisResult.h"
#include "core/application/workspace/AppState.h"

#include <map>

namespace core::analysis {

class AnalysisEngine::Impl {
public:
    Impl()
    {
        strategies.emplace(std::string(core::constants::analysis::kTypeTab), std::make_unique<TabAnalysisStrategy>());
        strategies.emplace(std::string(core::constants::analysis::kTypePlot), std::make_unique<PlotAnalysisStrategy>());
        strategies.emplace(std::string(core::constants::analysis::kTypeCalc), std::make_unique<CalcAnalysisStrategy>());
    }

    const AnalysisStrategy* resolveStrategy(const Analysis& analysis) const
    {
        const std::string key = analysis.type.empty() ? std::string(core::constants::analysis::kTypeTab) : analysis.type;
        const auto it = strategies.find(key);
        if (it != strategies.end()) return it->second.get();

        const auto fallback = strategies.find(std::string(core::constants::analysis::kTypeTab));
        return fallback != strategies.end() ? fallback->second.get() : nullptr;
    }

    std::map<std::string, std::unique_ptr<AnalysisStrategy>> strategies;
};

AnalysisEngine::AnalysisEngine()
    : impl_(std::make_unique<Impl>())
{
}

AnalysisEngine::~AnalysisEngine() = default;

AnalysisEngine::AnalysisEngine(AnalysisEngine&&) noexcept = default;

AnalysisEngine& AnalysisEngine::operator=(AnalysisEngine&&) noexcept = default;

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
    const AnalysisStrategy* strategy = impl_->resolveStrategy(analysis);
    AnalysisResult out;
    if (!strategy) return out;

    const Filter filter = parseFilterSpec(filterSpec);
    out = strategy->compute(analysis, state, filter);
    out.type = resolveAnalysisOutputType(analysis);
    out.configJson = analysis.configJson;
    out.transactions = collectTransactionsForAnalysis(state, filter);
    out.generatedAt = analysis.updatedAt.empty() ? analysis.createdAt : analysis.updatedAt;
    out.found = true;
    return out;
}

} // namespace core::analysis
