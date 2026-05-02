#pragma once

#include <string>

namespace core::domain {

class ExportLog {
public:
    std::string id;
    std::string time;
    std::string targetPath;
    std::string status;
    std::string message;
    std::string payload;
};

} // namespace core::domain
