/**
 * @file core/include/core/application/analysis/AnalysisFilterSpec.h
 * @brief Declares canonical parsing/building for analysis filter specs.
 */

#pragma once

#include <string>
#include <vector>

namespace core::application::analysis {

struct AnalysisFilterSelection {
    std::string dateField = "bookingDate";
    std::string dateMode = "year";
    std::string year;
    std::string dateFrom;
    std::string dateTo;
    std::vector<std::string> propertyIds;
    bool propertyIdsUnassigned = false;
    std::vector<std::string> contractTypes;
    bool contractTypesUnassigned = false;
    std::string allocatableMode = "all";
};

/**
 * @brief Parses one raw filter specification into structured selection fields.
 */
AnalysisFilterSelection parseAnalysisFilterSelection(const std::string& filterSpec);

/**
 * @brief Builds one canonical filter specification from structured selection fields.
 */
std::string buildAnalysisFilterSpec(const AnalysisFilterSelection& selection);

} // namespace core::application::analysis
