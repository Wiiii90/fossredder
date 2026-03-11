/**
 * @file core/include/core/models/Statement.h
 * @brief Domain model for statements.
 */

#pragma once

#include <string>

namespace core::domain {

class Statement {
public:
    Statement() = default;

    std::string id;
    std::string name;
};

}

using Statement = core::domain::Statement;
