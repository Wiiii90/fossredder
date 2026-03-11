/**
 * @file core/include/core/models/Contract.h
 * @brief Domain model for contracts.
 */

#pragma once

#include <string>
#include <vector>

namespace core::domain {

class Contract {
public:
    Contract() = default;

    std::string id;
    std::string name;
    std::string type;
    std::string description;
    std::string startDate;
    std::string endDate;
    double basePrice = 0.0;
    double consumptionPrice = 0.0;
    double monthlyAdvance = 0.0;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
};

}

using Contract = core::domain::Contract;
