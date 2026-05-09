/**
 * @file core/include/core/domain/entities/Actor.h
 * @brief Domain model for actors.
 */

#pragma once

#include <string>
#include <vector>

#include "core/domain/values/Alias.h"
#include "core/domain/values/AliasUsage.h"

namespace core::domain {

class Actor {
public:
    std::string id;
    std::string name;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;

    std::string createdAt;
    std::string updatedAt;
};

} // namespace core::domain
