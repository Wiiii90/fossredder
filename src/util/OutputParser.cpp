#include "util/OutputParser.h"
#include <regex>

std::unordered_map<std::string, std::string> OutputParser::parseTuplesToMap(const std::string& response) {
    std::unordered_map<std::string, std::string> result;
    std::regex tupleRegex(R"(\(\s*([^)]+?)\s*,\s*([^)]+?)\s*\))");
    std::smatch match;
    std::string::const_iterator searchStart(response.cbegin());
    while (std::regex_search(searchStart, response.cend(), match, tupleRegex)) {
        if (match.size() == 3) {
            result[match[1].str()] = match[2].str();
        }
        searchStart = match.suffix().first;
    }
    return result;
}
