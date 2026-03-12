#include "TabAnalysisStrategy.h"

#include "../FilteredTransactions.h"
#include "core/constants/CoreDefaults.h"

namespace core::analysis {

AnalysisResult TabAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const
{
    (void)analysis;
    AnalysisResult res;

    for (const auto& tptr : core::analysis::detail::collectFilteredTransactions(state, filterSpec)) {
        res.table.push_back({ tptr->bookingDate, tptr->name, std::to_string(tptr->amount) });
    }

    res.metrics[std::string(core::constants::analysis::metricKeys::kRowCount)] = static_cast<double>(res.table.size());
    return res;
}

}
