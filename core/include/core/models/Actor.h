/**
 * @file core/include/core/models/Actor.h
 * @brief Domain model for actors.
 */

#pragma once

#include <string>
#include <vector>

#include "core/models/Alias.h"
#include "core/models/AliasUsage.h"

namespace core::domain {

class Actor {
public:
    std::string id;
    std::string name;
    std::string createdAt;
    std::string updatedAt;
    std::vector<Alias> aliases;
    std::vector<AliasUsage> aliasUsage;
};

}
