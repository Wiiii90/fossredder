/**
 * @file core/include/core/domain/values/Alias.h
 * @brief Domain model for entity aliases.
 */

#pragma once

#include <string>

namespace core::domain {

struct Alias {
    std::string value;
    std::string kind;
    std::string source;
    std::string createdAt;
    std::string updatedAt;
};

} // namespace core::domain
