#include "core/application/AnalysisService.h"

#include "core/models/Analysis.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace core::application {

AnalysisResult AnalysisService::computeAnalysisById(const AppState& state,
                                                    const std::string& analysisId,
                                                    const std::string& filterSpec) const
{
    return engine_.computeAnalysisById(analysisId, state, filterSpec);
}

std::vector<std::string> AnalysisService::contractTypes(const AppState& state) const
{
    std::unordered_set<std::string> seen;
    std::vector<std::string> values;
    values.reserve(state.contracts.size());

    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        std::string type = contract->type;
        type.erase(type.begin(), std::find_if(type.begin(), type.end(), [](unsigned char c) { return std::isspace(c) == 0; }));
        type.erase(std::find_if(type.rbegin(), type.rend(), [](unsigned char c) { return std::isspace(c) == 0; }).base(), type.end());
        if (type.empty()) continue;
        if (!seen.insert(type).second) continue;
        values.push_back(std::move(type));
    }

    std::sort(values.begin(), values.end());
    return values;
}

}
