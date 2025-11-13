#pragma once
#include <string>
#include <unordered_map>

class OutputParser {
public:
    static std::unordered_map<std::string, std::string> parseTuplesToMap(const std::string& response);
};
