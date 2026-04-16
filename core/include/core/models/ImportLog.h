#pragma once

#include <string>

namespace core::domain {

class ImportLog {
public:
    std::string id;
    std::string time;
    std::string type;
    std::string file;
    std::string status;
    std::string message;
    bool draftAttached = false;
    std::string draftId;
    std::string statementId;
};

} // namespace core::domain
