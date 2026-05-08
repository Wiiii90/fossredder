/**
 * @file core/include/core/models/Analysis.h
 * @brief Domain model for persisted analysis definitions.
 */

#pragma once

#include <string>
#include <unordered_map>

namespace core::domain {

class Analysis {
public:
    Analysis() = default;

    std::string id;
    std::string name;
    std::string type;
    std::string configJson;
    std::string filterSpec;
    std::string exportFormat;
    bool includeCalcAdjustments = true;
    std::string exportStateJson;
    std::string snapshotTransactionsJson;
    std::unordered_map<std::string, double> adjustments;
    std::string createdAt;
    std::string updatedAt;
};

}

