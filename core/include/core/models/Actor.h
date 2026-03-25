/**
 * @file core/include/core/models/Actor.h
 * @brief Domain model for actors.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/models/AliasUsage.h"

namespace core::domain {

class Actor {
public:
    std::string id;
    std::string name;
    std::string type;
    std::string description;
    std::vector<std::string> aliases;
    std::vector<AliasUsage> aliasUsage;

    Actor() = default;
    Actor(std::string name, std::string type = {}, std::string description = {})
        : name(std::move(name)),
          type(std::move(type)),
          description(std::move(description)) {}
};

}
