/**
 * @file core/include/core/application/AnalysisRequestComposer.h
 * @brief Declares helpers for composing analysis creation and adjustment payloads.
 */

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core::domain {
struct AnalysisTransaction;
}

namespace core::application {

class AnalysisRequestComposer {
public:
    struct AdjustmentsJsonResult {
        std::unordered_map<std::string, double> adjustments;
        std::string error;
        bool valid = false;
    };

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
        const std::vector<core::domain::AnalysisTransaction>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent);

    static std::string buildTaxAdjustmentsJson(
        const std::vector<core::domain::AnalysisTransaction>& transactions,
        const std::vector<std::string>& selectedTransactionIds,
        double taxPercent);

    static std::string serializeAdjustments(const std::unordered_map<std::string, double>& adjustments);
    static AdjustmentsJsonResult parseAdjustmentsJson(const std::string& json);
};

} // namespace core::application
