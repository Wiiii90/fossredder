/**
 * @file core/src/application/analysis/ComposeAnalysisRequest.cpp
 * @brief Implements analysis request composition helpers used by the UI controllers.
 */

#include "core/pch.h"

#include "core/application/analysis/ComposeAnalysisRequest.h"

#include "core/constants/analysis.h"
#include "core/constants/filters.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace {

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string lowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string joinList(const std::vector<std::string>& values)
{
    std::string out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) out.push_back(core::constants::filters::separators::kList);
        out += values[i];
    }
    return out;
}

std::vector<std::string> normalizedCopy(const std::vector<std::string>& values)
{
    std::vector<std::string> out;
    out.reserve(values.size());
    for (const auto& value : values) {
        const auto trimmed = trim(value);
        if (!trimmed.empty()) out.push_back(trimmed);
    }
    return out;
}

bool containsTransactionId(const std::unordered_set<std::string>& selectedIds, const std::string& id)
{
    return selectedIds.find(id) != selectedIds.end();
}

} // namespace

namespace core::application::analysis {

RunAnalysisRequest ComposeAnalysisRequest::composeRunAnalysisRequest(const std::string& analysisId,
                                                                     const std::string& filterSpec)
{
    return RunAnalysisRequest{analysisId, filterSpec};
}

std::string ComposeAnalysisRequest::buildConfigJson(const std::string& type,
                                                    const std::string& plotType,
                                                    const std::string& plotMeasure,
                                                    const std::vector<std::string>& propertyIds,
                                                    const std::vector<std::string>& contractTypes,
                                                    double taxPercent)
{
    nlohmann::json config = nlohmann::json::object();
    const auto normalizedType = trim(type);

    if (normalizedType == core::constants::analysis::kTypeCalc) {
        config[core::constants::analysis::calc::kStrategyKey] = core::constants::analysis::calc::kStrategyTax;
        config[core::constants::analysis::calc::kPercentKey] = taxPercent;
    } else if (normalizedType == core::constants::analysis::kTypePlot) {
        config[core::constants::analysis::kPlotTypeKey] = trim(plotType).empty()
            ? std::string(core::constants::analysis::plotTypes::kPie)
            : trim(plotType);
        config[core::constants::analysis::kPlotMeasureKey] = trim(plotMeasure).empty()
            ? std::string(core::constants::analysis::plotMeasures::kTotalAmount)
            : trim(plotMeasure);
        config[core::constants::analysis::kPropertiesKey] = normalizedCopy(propertyIds);
        config[core::constants::analysis::kContractTypesKey] = normalizedCopy(contractTypes);
    }

    return config.dump();
}

std::string ComposeAnalysisRequest::buildFilterSpec(
    const std::string& dateMode,
    const std::string& year,
    const std::string& dateFrom,
    const std::string& dateTo,
    const std::vector<std::string>& propertyIds,
    const std::vector<std::string>& contractTypes,
    const std::string& allocatableMode)
{
    std::vector<std::string> clauses;

    const auto normalizedDateMode = lowerCopy(trim(dateMode));
    const auto normalizedYear = trim(year);
    const auto from = trim(dateFrom);
    const auto to = trim(dateTo);

    if (normalizedDateMode == "year" && !normalizedYear.empty()) {
        const std::string yearFrom = normalizedYear + "-01-01";
        const std::string yearTo = normalizedYear + "-12-31";
        clauses.push_back(std::string(core::constants::filters::kDate)
                          + std::string(core::constants::filters::operators::kGreaterEqual)
                          + yearFrom);
        clauses.push_back(std::string(core::constants::filters::kDate)
                          + std::string(core::constants::filters::operators::kLessEqual)
                          + yearTo);
    } else {
        if (!from.empty()) {
            clauses.push_back(std::string(core::constants::filters::kDate)
                              + std::string(core::constants::filters::operators::kGreaterEqual)
                              + from);
        }
        if (!to.empty()) {
            clauses.push_back(std::string(core::constants::filters::kDate)
                              + std::string(core::constants::filters::operators::kLessEqual)
                              + to);
        }
    }

    const auto normalizedPropertyIds = normalizedCopy(propertyIds);
    if (!normalizedPropertyIds.empty()) {
        clauses.push_back(std::string(core::constants::filters::kPropertyId)
                          + std::string(core::constants::filters::operators::kEqual)
                          + joinList(normalizedPropertyIds));
    }

    const auto normalizedContractTypes = normalizedCopy(contractTypes);
    if (!normalizedContractTypes.empty()) {
        clauses.push_back(std::string(core::constants::filters::kContractType)
                          + std::string(core::constants::filters::operators::kEqual)
                          + joinList(normalizedContractTypes));
    }

    const auto normalizedAllocatable = lowerCopy(trim(allocatableMode));
    if (normalizedAllocatable == "allocatable" || normalizedAllocatable == "non-allocatable") {
        clauses.push_back(std::string(core::constants::filters::kAllocatable)
                          + std::string(core::constants::filters::operators::kEqual)
                          + normalizedAllocatable);
    }

    std::string filterSpec;
    for (std::size_t i = 0; i < clauses.size(); ++i) {
        if (i > 0) filterSpec.push_back(core::constants::filters::separators::kClause);
        filterSpec += clauses[i];
    }

    return filterSpec;
}

std::unordered_map<std::string, double> ComposeAnalysisRequest::buildTaxAdjustments(
    const std::vector<AnalysisTransaction>& transactions,
    const std::vector<std::string>& selectedTransactionIds,
    double taxPercent)
{
    std::unordered_map<std::string, double> out;
    const auto selectedIds = std::unordered_set<std::string>(selectedTransactionIds.begin(), selectedTransactionIds.end());
    if (selectedIds.empty()) return out;

    const double factor = 1.0 + (taxPercent / 100.0);
    for (const auto& transaction : transactions) {
        if (transaction.id.empty()) continue;
        if (!containsTransactionId(selectedIds, transaction.id)) continue;
        out.emplace(transaction.id, transaction.amount * factor);
    }

    return out;
}

std::string ComposeAnalysisRequest::buildTaxAdjustmentsJson(
    const std::vector<AnalysisTransaction>& transactions,
    const std::vector<std::string>& selectedTransactionIds,
    double taxPercent)
{
    return serializeAdjustments(buildTaxAdjustments(transactions, selectedTransactionIds, taxPercent));
}

std::string ComposeAnalysisRequest::serializeAdjustments(const std::unordered_map<std::string, double>& adjustments)
{
    nlohmann::json obj = nlohmann::json::object();
    for (const auto& [id, amount] : adjustments) {
        obj[id] = amount;
    }
    return obj.dump();
}

ComposeAnalysisRequest::AdjustmentsJsonResult ComposeAnalysisRequest::parseAdjustmentsJson(const std::string& json)
{
    AdjustmentsJsonResult result;
    try {
        const auto doc = nlohmann::json::parse(json);
        if (!doc.is_object()) {
            result.error = "Expected JSON object";
            return result;
        }

        for (auto it = doc.begin(); it != doc.end(); ++it) {
            if (!it.value().is_number()) continue;
            result.adjustments.emplace(it.key(), it.value().get<double>());
        }

        result.valid = true;
        return result;
    } catch (const std::exception& ex) {
        result.error = ex.what();
    }

    return result;
}

} // namespace core::application::analysis
