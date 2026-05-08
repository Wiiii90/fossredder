/**
 * @file core/include/core/models/Annual.h
 * @brief Domain model for annual aggregates.
 */

#pragma once

#include <string>
#include <vector>

namespace core::domain {

class Annual {
public:
    Annual() = default;

    std::string id;
    std::string name;
    int year = 0;
    std::vector<std::string> analysisIds;

    std::string createdAt;
    std::string updatedAt;
};

}

