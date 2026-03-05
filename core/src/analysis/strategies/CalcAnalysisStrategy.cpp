#include "core/analysis/strategies/CalcAnalysisStrategy.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/Analysis.h"
#include "core/models/AppState.h"
#include "core/models/Transaction.h"
#include "core/analysis/Filter.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

using namespace std;

static pair<bool, double> parseTax(const Analysis& analysis)
{
    if (analysis.configJson.empty()) return { false, 1.0 };

    try {
        const auto config = nlohmann::json::parse(analysis.configJson);
        if (!config.is_object()) return { false, 1.0 };

        const auto strategyIt = config.find("strategy");
        if (strategyIt == config.end() || !strategyIt->is_string()) return { false, 1.0 };
        if (strategyIt->get<string>() != "tax") return { false, 1.0 };

        const auto percentIt = config.find("percent");
        if (percentIt == config.end() || !percentIt->is_number()) return { true, 1.0 };
        const double pct = percentIt->get<double>();
        return { true, 1.0 + (pct / 100.0) };
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      "core::analysis::CalcAnalysisStrategy::parseTax",
                                      std::current_exception());
    }

    return { false, 1.0 };
}

Analysis CalcAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    Analysis out;
    Filter f = parseFilterSpec(filterSpec);

    // merge explicit adjustments from Analysis.adjustments
    unordered_map<string,double> adjustments = analysis.adjustments;

    const auto [hasTax, taxFactor] = parseTax(analysis);

    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        if (!filterSpec.empty() && !f.matches(tptr, state)) continue;

        // label (use bookingDate or name)
        std::string label = tptr->bookingDate.empty() ? tptr->name : tptr->bookingDate;

        double adjusted = tptr->amount;
        auto it = adjustments.find(tptr->id);
        if (it != adjustments.end()) adjusted = it->second;
        else if (hasTax) adjusted = adjusted * taxFactor;

        nlohmann::json summary;
        summary["amount_original"] = tptr->amount;
        summary["amount_adjusted"] = adjusted;
        if (hasTax) {
            summary["taxPercent"] = (taxFactor - 1.0) * 100.0;
            summary["taxFactor"] = taxFactor;
        }
        summary["txId"] = tptr->id;

        out.table.push_back({ label, summary.dump() });
    }

    out.createdAt = "";
    out.metrics["rows"] = static_cast<double>(out.table.size());
    return out;
}
