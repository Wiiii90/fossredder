#pragma once

#include <string>

class Property {
public:
    std::string id;
    std::string name;
    std::string address;
    std::string description;

    double consumption = 0.0;
    std::string consumptionUnit;

    Property() = default;
    Property(const std::string& name, const std::string& address, const std::string& description = "");
};