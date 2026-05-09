/**
 * @file core/include/core/domain/entities/Contract.h
 * @brief Domain model for contracts.
 */

#pragma once

#include <string>
#include <vector>

#include "core/domain/values/Alias.h"
#include "core/domain/values/AliasUsage.h"

namespace core::domain {

class Contract {
public:
    Contract() = default;

    std::string id;
    std::string name;
    std::string type;
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;

    std::string createdAt;
    std::string updatedAt;
};

} // namespace core::domain
