/**
 * @file core/include/core/jobs/ImportJobSpec.h
 * @brief Declares the public import job submission contract.
 */

#pragma once

#include <string>

namespace core::jobs {

struct ImportStatementJobSpec {
    std::string sourcePath;
    std::string runRoot;
    std::string runIdPrefix;
};

}
