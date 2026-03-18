/**
 * @file core/src/utils/Util.h
 * @brief Declares private string helpers shared across `core` implementation files.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace utils {

std::string trim(std::string s);
std::string lowerAscii(std::string s);
std::string collapseWhitespace(std::string s);
std::vector<std::string> splitWhitespace(const std::string& s);
int rightEdgeFromWordSpans(const std::vector<std::pair<int, int>>& wordSpans);

} // namespace utils
