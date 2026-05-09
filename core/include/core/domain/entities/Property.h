/**
 * @file core/include/core/domain/entities/Property.h
 * @brief Domain model for properties.
 */

#pragma once

#include <string>
#include <vector>

#include "core/domain/values/Alias.h"
#include "core/domain/values/AliasUsage.h"

namespace core::domain {

class Property {
public:
    std::string id;
    std::string name;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;

    std::string createdAt;
    std::string updatedAt;
};

} // namespace core::domain
