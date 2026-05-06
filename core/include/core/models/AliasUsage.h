/**
 * @file core/include/core/models/AliasUsage.h
 * @brief Persistent alias usage metadata for ranked matching and suggestions.
 */

#pragma once


#include <string>

#include "core/models/Alias.h"

namespace core::domain {

struct AliasUsage {
    Alias alias;
    int hitCount = 0;
    std::string lastUsedAt;
    std::string updatedAt;
    std::string createdAt;
};

} // namespace core::domain
