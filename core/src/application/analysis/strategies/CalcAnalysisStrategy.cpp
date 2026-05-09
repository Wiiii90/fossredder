#include "CalcAnalysisStrategy.h"

#include "../FilteredTransactions.h"
#include "core/constants/analysis.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/domain/entities/Analysis.h"
#include "core/application/workspace/AppState.h"

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>

namespace {

struct TaxConfig {
    bool enabled = false;
    double factor = 1.0;
};

TaxConfig parseTax(const Analysis& analysis)
{
    TaxConfig config;
    if (analysis.configJson.empty()) return config;

    try {
        const auto json = nlohmann::json::parse(analysis.configJson);
        if (!json.is_object()) return config;

        const auto strategyIt = json.find(core::constants::analysis::calc::kStrategyKey);
        if (strategyIt == json.end() || !strategyIt->is_string()) return config;
        if (strategyIt->get<std::string>() != core::constants::analysis::calc::kStrategyTax) return config;

        config.enabled = true;

        const auto percentIt = json.find(core::constants::analysis::calc::kPercentKey);
        if (percentIt == json.end() || !percentIt->is_number()) return config;

        const double percent = percentIt->get<double>();
        config.factor = 1.0 + (percent / 100.0);
        return config;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      "core::analysis::CalcAnalysisStrategy::parseTax",
                                      std::current_exception());
    }

    return {};
}

}

namespace core::analysis {

AnalysisResult CalcAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const Filter& filter) const {
    AnalysisResult out;
    const std::unordered_map<std::string, double> adjustments = analysis.adjustments;
    const TaxConfig taxConfig = parseTax(analysis);

    for (const auto& transaction : core::analysis::detail::collectFilteredTransactions(state, filter)) {

        const std::string label = transaction->bookingDate.empty() ? transaction->name : transaction->bookingDate;

        double adjustedAmount = transaction->amount;
        const auto adjustmentIt = adjustments.find(transaction->id);
        if (adjustmentIt != adjustments.end()) adjustedAmount = adjustmentIt->second;
        else if (taxConfig.enabled) adjustedAmount *= taxConfig.factor;

        nlohmann::json summary;
        summary[core::constants::analysis::resultFields::kAmountOriginal] = transaction->amount;
        summary[core::constants::analysis::resultFields::kAmountAdjusted] = adjustedAmount;
        if (taxConfig.enabled) {
            summary[core::constants::analysis::resultFields::kTaxPercent] = (taxConfig.factor - 1.0) * 100.0;
            summary[core::constants::analysis::resultFields::kTaxFactor] = taxConfig.factor;
        }
        summary[core::constants::analysis::resultFields::kTransactionId] = transaction->id;

        out.table.push_back({ label, summary.dump() });
    }

    out.metrics[std::string(core::constants::analysis::metricKeys::kRows)] = static_cast<double>(out.table.size());
    return out;
}

}
