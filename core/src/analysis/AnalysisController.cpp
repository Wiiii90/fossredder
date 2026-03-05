#include "core/analysis/AnalysisController.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "core/models/Analysis.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"
#include "core/analysis/strategies/TabAnalysisStrategy.h"
#include "core/analysis/strategies/PlotAnalysisStrategy.h"
#include "core/analysis/strategies/CalcAnalysisStrategy.h"
#include "core/analysis/Filter.h"

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
                                      "core::analysis::AnalysisController::resolveOutputType",
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
    for (const auto& cptr : state.contracts) {
        if (!cptr) continue;
        contractById.emplace(cptr->id, cptr);
    }

    Filter f = parseFilterSpec(filterSpec);

    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        if (!filterSpec.empty() && !f.matches(tptr, state)) continue;

        auto tx = std::make_shared<Transaction>(*tptr);
        tx->contract = nullptr;
        if (!tx->contractId.empty()) {
            const auto it = contractById.find(tx->contractId);
            if (it != contractById.end() && it->second) tx->contract = it->second.get();
        }
        out.push_back(std::move(tx));
    }

    return out;
}

}

AnalysisController::AnalysisController()
{
    // register built-in strategies
    strategies_.emplace("tab", std::make_unique<TabAnalysisStrategy>());
    strategies_.emplace("plot", std::make_unique<PlotAnalysisStrategy>());
    strategies_.emplace("calc", std::make_unique<CalcAnalysisStrategy>());
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

Analysis AnalysisController::computeAnalysisById(const std::string& analysisId, const AppState& state, const std::string& filterSpec) const {
    for (const auto& a : state.analyses) {
        if (!a) continue;
        if (a->id != analysisId) continue;
        const std::string effectiveFilter = filterSpec.empty() ? a->filterSpec : filterSpec;
        return computeAnalysis(*a, state, effectiveFilter);
    }
    return Analysis{};
}

Analysis AnalysisController::computeAnalysis(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    const IAnalysisStrategy* strat = resolveStrategy(analysis);
    Analysis out;
    if (strat) out = strat->compute(analysis, state, filterSpec);
    out.type = resolveOutputType(analysis);
    out.configJson = analysis.configJson;
    out.transactions = collectTransactions(state, filterSpec);
    out.found = true;
    return out;
}
