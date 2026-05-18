/**
 * @file core/include/core/application/analysis/AnalysisResult.h
 * @brief Declares the canonical result models produced by analysis execution.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace core::application::analysis {

/**
 * @brief Serialized transaction row included in analysis result payloads.
 */
struct AnalysisTransaction {
    std::string id;
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string contractId;
    std::string contractType;
    std::vector<std::string> propertyIds;
    std::vector<std::string> propertyNames;
};

/**
 * @brief Materialized output of one analysis execution.
 */
struct AnalysisResult {
    std::string type;
    std::string configJson;
    std::map<std::string, double> metrics;
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> artifacts;
    std::vector<AnalysisTransaction> transactions;
    bool found = false;
    std::string generatedAt;
};

} // namespace core::application::analysis

namespace core::domain {
using AnalysisTransaction = core::application::analysis::AnalysisTransaction;
using AnalysisResult = core::application::analysis::AnalysisResult;
}
