#pragma once

#include <string>
#include <vector>

namespace core::domain {

class ExportLog {
public:
    std::string id;
    std::string time;
    std::string targetPath;
    std::string status;
    std::string message;
    std::string payload;
    std::vector<std::string> annualIds;
    std::vector<std::string> analysisIds;
};

} // namespace core::domain
