#pragma once
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace utils {

std::string trim(std::string s);
std::string lowerAscii(std::string s);

std::string collapseWhitespace(std::string s);
std::vector<std::string> splitWhitespace(const std::string& s);

int rightEdgeFromWordSpans(const std::vector<std::pair<int, int>>& wordSpans);

} // namespace utils
