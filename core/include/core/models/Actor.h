/**
 * @file core/include/core/models/Actor.h
 * @brief Domain model for actors.
 */

#pragma once

#include <string>
#include <utility>

namespace core::domain {

class Actor {
public:
    std::string id;
    std::string name;
    std::string type;
    std::string description;

    Actor() = default;
    Actor(std::string name, std::string type = {}, std::string description = {})
        : name(std::move(name)),
          type(std::move(type)),
          description(std::move(description)) {}
};

}
