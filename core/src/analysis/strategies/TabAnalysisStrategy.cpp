#include "core/analysis/strategies/TabAnalysisStrategy.h"

#include "core/models/Transaction.h"
#include "core/analysis/Filter.h"
#include "core/constants/CoreDefaults.h"

AnalysisResult TabAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const
{
    AnalysisResult res;

    // Naive implementation: iterate all transactions and include those matching filterSpec
    core::analysis::Filter f = core::analysis::parseFilterSpec(filterSpec);
    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        if (!filterSpec.empty() && !f.matches(tptr, state)) continue;
        res.table.push_back({ tptr->bookingDate, tptr->name, std::to_string(tptr->amount) });
    }

    res.metrics[std::string(core::constants::analysis::metricKeys::kRowCount)] = static_cast<double>(res.table.size());
    return res;
}
