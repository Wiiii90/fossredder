#pragma once

#include <map>
#include <string>
#include <vector>

namespace core::domain {

struct AnalysisTransaction {
    std::string id;
    std::string name;
    std::string bookingDate;
    double amount = 0.0;
    std::string contractId;
    std::string contractType;
    std::vector<std::string> propertyIds;
};

class AnalysisResult {
public:
    std::string type;
    std::string configJson;
    std::map<std::string, double> metrics;
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> artifacts;
    std::vector<AnalysisTransaction> transactions;
    bool found = false;
    std::string generatedAt;
};

}

