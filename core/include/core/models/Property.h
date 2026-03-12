/**
 * @file core/include/core/models/Property.h
 * @brief Domain model for properties.
 */

#pragma once

#include <string>
#include <utility>

namespace core::domain {

class Property {
public:
    std::string id;
    std::string name;
    std::string address;
    std::string description;
    double consumption = 0.0;
    std::string consumptionUnit;

    Property() = default;
    Property(std::string name, std::string address, std::string description = {})
        : name(std::move(name)),
          address(std::move(address)),
          description(std::move(description)) {}
};

}
