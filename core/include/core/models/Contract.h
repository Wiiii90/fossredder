/**
 * @file core/include/core/models/Contract.h
 * @brief Domain model for contracts.
 */

#pragma once

#include <string>
#include <vector>

#include "core/models/Alias.h"
#include "core/models/AliasUsage.h"

namespace core::domain {

class Contract {
public:
    Contract() = default;

    std::string id;
    std::string name;
    std::string type;
    std::string createdAt;
    std::string updatedAt;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;
};

}

