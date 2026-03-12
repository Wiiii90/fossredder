/**
 * @file core/src/analysis/strategies/PlotAnalysisStrategy.cpp
 * @brief Implements the private plot analysis strategy.
 */

#include "PlotAnalysisStrategy.h"

#include "core/analysis/Filter.h"
#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"
#include "core/utils/Util.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace {

using ContractTypeIndex = std::unordered_map<std::string, std::string>;

struct PlotConfig {
    std::string plotType = std::string(core::constants::analysis::plotTypes::kPie);
    std::string plotMeasure = std::string(core::constants::analysis::plotMeasures::kTotalAmount);
    std::vector<std::string> propertyFilter;
    std::vector<std::string> contractTypeFilter;
};

std::string normalizeValue(const std::string& value)
{
    std::string normalized = utils::trim(value);
    if (normalized.empty()) return {};
    for (auto& ch : normalized)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return normalized;
}

std::vector<std::string> normalizeList(const std::vector<std::string>& values)
{
    std::vector<std::string> normalizedValues;
    normalizedValues.reserve(values.size());
    for (const auto& value : values) {
        const auto normalized = normalizeValue(value);
        if (!normalized.empty()) normalizedValues.push_back(normalized);
    }
    return normalizedValues;
}

std::string normalizePlotMeasure(const std::string& value)
{
    const auto normalized = normalizeValue(value);
    if (normalized == core::constants::analysis::plotMeasures::kCount) return std::string(core::constants::analysis::plotMeasures::kCount);
    if (normalized == "averageamount" || normalized == "average amount") return std::string(core::constants::analysis::plotMeasures::kAverageAmount);
    if (normalized == "totalamount" || normalized == "total amount") return std::string(core::constants::analysis::plotMeasures::kTotalAmount);
    return normalized;
}

ContractTypeIndex buildContractTypeIndex(const AppState& state, bool normalize)
{
    ContractTypeIndex index;
    index.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        index.emplace(contract->id, normalize ? normalizeValue(contract->type) : contract->type);
    }
    return index;
}

bool matchesPropertyFilter(const Transaction& transaction, const std::vector<std::string>& propertyFilter)
{
    if (propertyFilter.empty()) return true;

    for (const auto& propertyId : transaction.propertyIds) {
        if (std::find(propertyFilter.begin(), propertyFilter.end(), propertyId) != propertyFilter.end()) {
            return true;
        }
    }

    return false;
}

bool matchesContractTypeFilter(const Transaction& transaction,
                               const std::vector<std::string>& normalizedContractTypeFilter,
                               const ContractTypeIndex& normalizedContractTypeById)
{
    if (normalizedContractTypeFilter.empty()) return true;

    if (transaction.contractId.empty()) {
        return std::find(normalizedContractTypeFilter.begin(),
                         normalizedContractTypeFilter.end(),
                         std::string(core::constants::filters::kUnassigned)) != normalizedContractTypeFilter.end();
    }

    const auto it = normalizedContractTypeById.find(transaction.contractId);
    if (it == normalizedContractTypeById.end()) return false;

    return std::find(normalizedContractTypeFilter.begin(),
                     normalizedContractTypeFilter.end(),
                     it->second) != normalizedContractTypeFilter.end();
}

std::vector<std::shared_ptr<Transaction>> collectMatchedTransactions(const AppState& state,
                                                                     const std::string& filterSpec,
                                                                     const std::vector<std::string>& propertyFilter,
                                                                     const std::vector<std::string>& normalizedContractTypeFilter,
                                                                     const ContractTypeIndex& normalizedContractTypeById)
{
    core::analysis::Filter filter = core::analysis::parseFilterSpec(filterSpec);

    std::vector<std::shared_ptr<Transaction>> matched;
    matched.reserve(state.transactions.size());
    for (const auto& transaction : state.transactions) {
        if (!transaction) continue;
        if (!filterSpec.empty() && !filter.matches(transaction, state)) continue;
        if (!matchesPropertyFilter(*transaction, propertyFilter)) continue;
        if (!matchesContractTypeFilter(*transaction, normalizedContractTypeFilter, normalizedContractTypeById)) continue;
        matched.push_back(transaction);
    }

    return matched;
}

std::string extractYearMonth(const std::string& bookingDate)
{
    const std::string value = utils::trim(bookingDate);
    if (value.empty()) return {};

    if (value.size() >= 7
        && std::isdigit(static_cast<unsigned char>(value[0]))
        && std::isdigit(static_cast<unsigned char>(value[1]))
        && std::isdigit(static_cast<unsigned char>(value[2]))
        && std::isdigit(static_cast<unsigned char>(value[3]))) {
        const std::string year = value.substr(0, 4);
        if (value[4] == '-') {
            return year + "-" + value.substr(5, 2);
        }

        if (value.size() >= 6) {
            return year + "-" + value.substr(4, 2);
        }
    }

    if (value.find('.') != std::string::npos) {
        std::vector<std::string> parts;
        std::istringstream stream(value);
        std::string token;
        while (std::getline(stream, token, '.')) {
            parts.push_back(utils::trim(token));
        }

        if (parts.size() == 3) {
            std::string month = parts[1];
            if (parts[2].size() == 4 && (month.size() == 1 || month.size() == 2)) {
                if (month.size() == 1) month.insert(month.begin(), '0');
                return parts[2] + "-" + month;
            }
        }
    }

    return value;
}

PlotConfig parsePlotConfig(const Analysis& analysis)
{
    PlotConfig config;
    if (analysis.configJson.empty()) return config;

    try {
        const auto json = nlohmann::json::parse(analysis.configJson);
        if (json.contains(core::constants::analysis::kPlotTypeKey)) {
            config.plotType = json[core::constants::analysis::kPlotTypeKey].get<std::string>();
        }
        if (json.contains(core::constants::analysis::kPlotMeasureKey)) {
            config.plotMeasure = json[core::constants::analysis::kPlotMeasureKey].get<std::string>();
        }
        if (json.contains(core::constants::analysis::kPropertiesKey) && json[core::constants::analysis::kPropertiesKey].is_array()) {
            for (const auto& property : json[core::constants::analysis::kPropertiesKey]) {
                config.propertyFilter.push_back(property.get<std::string>());
            }
        }
        if (json.contains(core::constants::analysis::kContractTypesKey) && json[core::constants::analysis::kContractTypesKey].is_array()) {
            for (const auto& contractType : json[core::constants::analysis::kContractTypesKey]) {
                config.contractTypeFilter.push_back(contractType.get<std::string>());
            }
        }
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning,
                                      "core::analysis::PlotAnalysisStrategy::parseConfig",
                                      std::current_exception());
    }

    config.plotType = normalizeValue(config.plotType);
    config.plotMeasure = normalizePlotMeasure(config.plotMeasure);
    return config;
}

AnalysisResult buildPieResult(const std::vector<std::shared_ptr<Transaction>>& matched,
                              const ContractTypeIndex& normalizedContractTypeById,
                              const std::string& plotMeasure)
{
    AnalysisResult result;
    std::map<std::string, double> aggregatedAmounts;
    std::map<std::string, int> transactionCounts;

    for (const auto& transaction : matched) {
        std::string key = std::string(core::constants::filters::kUnassigned);
        if (!transaction->contractId.empty()) {
            const auto it = normalizedContractTypeById.find(transaction->contractId);
            if (it != normalizedContractTypeById.end() && !it->second.empty()) key = it->second;
            else key = transaction->contractId;
        }

        aggregatedAmounts[key] += transaction->amount;
        transactionCounts[key]++;
    }

    double totalAmount = 0.0;
    std::size_t totalCount = 0;
    for (const auto& [key, amount] : aggregatedAmounts) {
        totalAmount += std::fabs(amount);
        totalCount += transactionCounts[key];
    }

    for (const auto& [key, amount] : aggregatedAmounts) {
        double value = 0.0;
        if (plotMeasure == core::constants::analysis::plotMeasures::kCount) {
            value = static_cast<double>(transactionCounts[key]);
        } else if (plotMeasure == core::constants::analysis::plotMeasures::kAverageAmount) {
            value = transactionCounts[key] > 0 ? std::fabs(amount) / transactionCounts[key] : 0.0;
        } else {
            value = std::fabs(amount);
        }

        result.table.push_back({key, std::to_string(value)});
    }

    result.metrics[std::string(core::constants::analysis::metricKeys::kTotalAmount)] = totalAmount;
    result.metrics[std::string(core::constants::analysis::metricKeys::kRowCount)] = static_cast<double>(totalCount);
    return result;
}

AnalysisResult buildHistogramResult(const std::vector<std::shared_ptr<Transaction>>& matched,
                                    const AppState& state)
{
    AnalysisResult result;
    if (matched.empty()) return result;

    const auto contractTypeByIdRaw = buildContractTypeIndex(state, false);
    std::map<std::string, double> monthTotal;
    std::map<std::string, std::map<std::string, double>> monthByContract;
    std::map<std::string, std::map<std::string, double>> monthByProperty;

    for (const auto& transaction : matched) {
        const std::string month = extractYearMonth(transaction->bookingDate);
        const double amount = std::fabs(transaction->amount);
        monthTotal[month] += amount;

        std::string contractType = std::string(core::constants::filters::kUnassigned);
        if (!transaction->contractId.empty()) {
            const auto it = contractTypeByIdRaw.find(transaction->contractId);
            if (it != contractTypeByIdRaw.end() && !it->second.empty()) contractType = it->second;
            else contractType = transaction->contractId;
        }
        monthByContract[month][contractType] += amount;

        if (!transaction->propertyIds.empty()) {
            for (const auto& propertyId : transaction->propertyIds) {
                monthByProperty[month][propertyId] += amount;
            }
        } else {
            monthByProperty[month][std::string(core::constants::analysis::labels::kNoProperty)] += amount;
        }
    }

    std::vector<std::string> months;
    months.reserve(monthTotal.size());
    for (const auto& [month, amount] : monthTotal) {
        (void)amount;
        months.push_back(month);
    }
    std::sort(months.begin(), months.end());

    double totalAmount = 0.0;
    for (const auto& month : months) {
        nlohmann::json summary;
        summary[core::constants::analysis::resultFields::kMonth] = month;
        summary[core::constants::analysis::resultFields::kTotal] = monthTotal[month];
        summary[core::constants::analysis::resultFields::kByContract] = nlohmann::json::object();
        for (const auto& [label, amount] : monthByContract[month]) {
            summary[core::constants::analysis::resultFields::kByContract][label] = amount;
        }
        summary[core::constants::analysis::resultFields::kByProperty] = nlohmann::json::object();
        for (const auto& [label, amount] : monthByProperty[month]) {
            summary[core::constants::analysis::resultFields::kByProperty][label] = amount;
        }

        result.table.push_back({month, summary.dump()});
        totalAmount += monthTotal[month];
    }

    result.metrics[std::string(core::constants::analysis::metricKeys::kTotalAmount)] = totalAmount;
    result.metrics[std::string(core::constants::analysis::metricKeys::kRowCount)] = static_cast<double>(months.size());
    result.metrics[std::string(core::constants::analysis::metricKeys::kMatchedTransactions)] = static_cast<double>(matched.size());
    return result;
}

}

namespace core::analysis {

AnalysisResult PlotAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    const PlotConfig config = parsePlotConfig(analysis);
    const auto normalizedContractTypeFilter = normalizeList(config.contractTypeFilter);
    const auto normalizedContractTypeById = buildContractTypeIndex(state, true);
    const auto matched = collectMatchedTransactions(state,
                                                   filterSpec,
                                                    config.propertyFilter,
                                                   normalizedContractTypeFilter,
                                                   normalizedContractTypeById);

    if (config.plotType == core::constants::analysis::plotTypes::kPie) {
        return buildPieResult(matched, normalizedContractTypeById, config.plotMeasure);
    }
    if (config.plotType == core::constants::analysis::plotTypes::kHistogram) {
        return buildHistogramResult(matched, state);
    }

    return {};
}

}

