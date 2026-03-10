#include "core/analysis/AnalysisEngine.h"

#include "core/analysis/Filter.h"
#include "core/analysis/strategies/CalcAnalysisStrategy.h"
#include "core/analysis/strategies/PlotAnalysisStrategy.h"
#include "core/analysis/strategies/TabAnalysisStrategy.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/Analysis.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <nlohmann/json.hpp>

#include <unordered_map>

namespace {

std::string resolveOutputType(const Analysis& analysis)
{
    if (analysis.type != "plot" || analysis.configJson.empty()) return analysis.type;

    try {
        const auto config = nlohmann::json::parse(analysis.configJson);
        if (config.contains("plotType") && config["plotType"].is_string()) {
            return config["plotType"].get<std::string>();
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      core::errors::codes::ExceptionError,
                                      "core::analysis::AnalysisEngine::resolveOutputType",
                                      std::current_exception());
    }

    return analysis.type;
}

std::vector<std::shared_ptr<Transaction>> collectTransactions(const AppState& state, const std::string& filterSpec)
{
    std::vector<std::shared_ptr<Transaction>> out;
    out.reserve(state.transactions.size());

    std::unordered_map<std::string, std::shared_ptr<Contract>> contractById;
    contractById.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        contractById.emplace(contract->id, contract);
    }

    Filter filter = parseFilterSpec(filterSpec);

    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (!filterSpec.empty() && !filter.matches(transaction, state)) continue;

        auto copy = std::make_shared<Transaction>(*transaction);
        copy->contract = nullptr;
        if (!copy->contractId.empty()) {
            const auto it = contractById.find(copy->contractId);
            if (it != contractById.end() && it->second) copy->contract = it->second.get();
        }
        out.push_back(std::move(copy));
    }

    return out;
}

}

AnalysisEngine::AnalysisEngine()
{
    strategies_.emplace("tab", std::make_unique<TabAnalysisStrategy>());
    strategies_.emplace("plot", std::make_unique<PlotAnalysisStrategy>());
    strategies_.emplace("calc", std::make_unique<CalcAnalysisStrategy>());
}

AnalysisEngine::~AnalysisEngine() = default;

const IAnalysisStrategy* AnalysisEngine::resolveStrategy(const Analysis& analysis) const
{
    const std::string key = analysis.type.empty() ? "tab" : analysis.type;
    const auto it = strategies_.find(key);
    if (it != strategies_.end()) return it->second.get();

    const auto fallback = strategies_.find("tab");
    return fallback != strategies_.end() ? fallback->second.get() : nullptr;
}

Analysis AnalysisEngine::computeAnalysisById(const std::string& analysisId,
                                            const AppState& state,
                                            const std::string& filterSpec) const
{
    for (const auto& analysis : state.analyses) {
        if (!analysis) continue;
        if (analysis->id != analysisId) continue;
        const std::string effectiveFilter = filterSpec.empty() ? analysis->filterSpec : filterSpec;
        return computeAnalysis(*analysis, state, effectiveFilter);
    }

    return Analysis{};
}

Analysis AnalysisEngine::computeAnalysis(const Analysis& analysis,
                                         const AppState& state,
                                         const std::string& filterSpec) const
{
    const IAnalysisStrategy* strategy = resolveStrategy(analysis);
    Analysis out;
    if (strategy) out = strategy->compute(analysis, state, filterSpec);
    out.type = resolveOutputType(analysis);
    out.configJson = analysis.configJson;
    out.transactions = collectTransactions(state, filterSpec);
    out.found = true;
    return out;
}
