#pragma once

#include <string>
#include <vector>
#include <map>

class AnalysisResult {
public:
    AnalysisResult() = default;

    // simple scalar metrics
    std::map<std::string, double> metrics;

    // tabular data as rows of string columns
    std::vector<std::vector<std::string>> table;

    // optional file paths produced by analysis (e.g. plot images)
    std::vector<std::string> artifacts;

    std::string generatedAt;
};
