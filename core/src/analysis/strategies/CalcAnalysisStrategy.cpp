#include "core/analysis/strategies/CalcAnalysisStrategy.h"
#include "core/models/Analysis.h"
#include "core/models/AppState.h"
#include "core/models/Transaction.h"
#include "core/analysis/AnalysisResult.h"
#include "core/analysis/Filter.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <sstream>
#include <cctype>

using namespace std;

static optional<double> parseJsonNumberByKey(const std::string& json, const std::string& key) {
    const std::string qkey = '"' + key + '"';
    auto pos = json.find(qkey);
    if (pos == std::string::npos) return {};
    auto colon = json.find(':', pos + qkey.size());
    if (colon == std::string::npos) return {};
    size_t i = colon + 1;
    while (i < json.size() && isspace(static_cast<unsigned char>(json[i]))) ++i;
    if (i < json.size() && (json[i] == '"' || json[i] == '\'')) ++i;
    size_t start = i;
    while (i < json.size() && (isdigit(static_cast<unsigned char>(json[i])) || json[i]=='+' || json[i]=='-' || json[i]=='.' || json[i]=='e' || json[i]=='E')) ++i;
    if (start == i) return {};
    try { return stod(json.substr(start, i - start)); } catch (...) { return {}; }
}

AnalysisResult CalcAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    AnalysisResult out;
    Filter f = parseFilterSpec(filterSpec);

    // merge explicit adjustments from Analysis.adjustments
    unordered_map<string,double> adjustments = analysis.adjustments;

    // detect simple tax strategy from configJson
    bool hasTax = false;
    double taxFactor = 1.0;
    if (!analysis.configJson.empty()) {
        auto spos = analysis.configJson.find("\"strategy\"");
        if (spos != string::npos) {
            auto tag = analysis.configJson.substr(spos, 64);
            if (tag.find("tax") != string::npos) {
                auto pct = parseJsonNumberByKey(analysis.configJson, "percent");
                if (pct) { taxFactor = 1.0 + (*pct / 100.0); hasTax = true; }
            }
        }
    }

    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        if (!filterSpec.empty() && !f.matches(tptr, state)) continue;

        // label (use bookingDate or name)
        std::string label = tptr->bookingDate.empty() ? tptr->name : tptr->bookingDate;

        double adjusted = tptr->amount;
        auto it = adjustments.find(tptr->id);
        if (it != adjustments.end()) adjusted = it->second;
        else if (hasTax) adjusted = adjusted * taxFactor;

        // JSON summary as second column
        std::ostringstream ss;
        ss << "{\"amount_original\":" << tptr->amount << ",\"amount_adjusted\":" << adjusted;
        if (hasTax) ss << ",\"taxPercent\":" << (taxFactor - 1.0) * 100.0 << ",\"taxFactor\":" << taxFactor;
        ss << ",\"txId\":\"" << tptr->id << "\"}";

        out.table.push_back({ label, ss.str() });
    }

    out.generatedAt = "";
    out.metrics["rows"] = static_cast<double>(out.table.size());
    return out;
}
