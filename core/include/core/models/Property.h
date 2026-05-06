/**
 * @file core/include/core/models/Property.h
 * @brief Domain model for properties.
 */

#pragma once

#include <string>
#include <vector>

#include "core/models/Alias.h"
#include "core/models/AliasUsage.h"

namespace core::domain {

class Property {
public:
    std::string id;
    std::string name;
    std::string createdAt;
    std::string updatedAt;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;
};

}
