#include "core/analysis/strategies/TabAnalysisStrategy.h"
#include "core/analysis/AnalysisResult.h"
#include "core/models/Transaction.h"
#include "core/analysis/Filter.h"

AnalysisResult TabAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const
{
    AnalysisResult res;
    res.generatedAt = "";

    // Naive implementation: iterate all transactions and include those matching filterSpec
    Filter f = parseFilterSpec(filterSpec);
    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        if (!filterSpec.empty() && !f.matches(tptr, state)) continue;
        res.table.push_back({ tptr->bookingDate, tptr->name, std::to_string(tptr->amount) });
    }

    // provide a simple metric
    res.metrics["rowCount"] = static_cast<double>(res.table.size());
    return res;
}
