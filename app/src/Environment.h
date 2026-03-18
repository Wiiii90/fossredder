/**
 * @file app/src/Environment.h
 * @brief Declares application-local environment bootstrap helpers.
 */

#pragma once

#include <string>

namespace app::runtime {

bool loadDotEnv(const std::string& path, bool overwrite = false);

} // namespace app::runtime
