#pragma once

#include "core/application/analysis/RunAnalysisRequest.h"
#include "core/application/analysis/RunAnalysisResult.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::application::analysis {

class ComposeAnalysisRequest {
public:
    struct AdjustmentsJsonResult {
        std::unordered_map<std::string, double> adjustments;
        std::string error;
        bool valid = false;
    };

    static RunAnalysisRequest composeRunAnalysisRequest(const std::string& analysisId,
                                                        const std::string& filterSpec = {});

    static std::string buildConfigJson(const std::string& type,
                                       const std::string& plotType,
                                       const std::string& plotMeasure,
                                       const std::vector<std::string>& propertyIds,
                                       const std::vector<std::string>& contractTypes,
                                       double taxPercent = 0.0);

    static std::string buildFilterSpec(const std::string& dateMode,
                                       const std::string& year,
                                       const std::string& dateFrom,
                                       const std::string& dateTo,
                                       const std::vector<std::string>& propertyIds,
                                       const std::vector<std::string>& contractTypes,
                                       const std::string& allocatableMode);

    static std::unordered_map<std::string, double> buildTaxAdjustments(
        const std::vector<AnalysisTransaction>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent);

    static std::string buildTaxAdjustmentsJson(
        const std::vector<AnalysisTransaction>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent);

    static std::string serializeAdjustments(const std::unordered_map<std::string, double>& adjustments);
    static AdjustmentsJsonResult parseAdjustmentsJson(const std::string& json);
};

} // namespace core::application::analysis

namespace core::application {
using AnalysisRequestComposer = analysis::ComposeAnalysisRequest;
}
