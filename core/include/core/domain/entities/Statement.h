/**
 * @file core/include/core/domain/entities/Statement.h
 * @brief Domain model for statements.
 */

#pragma once

#include <string>
#include <vector>

namespace core::domain {

class Statement {
public:
    Statement() = default;

    std::string id;
    std::string name;
    std::vector<std::string> transactionIds;
    std::string createdAt;
    std::string updatedAt;
};

} // namespace core::domain
