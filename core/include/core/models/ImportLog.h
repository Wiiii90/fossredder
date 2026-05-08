#pragma once

#include <string>
#include <vector>

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
    std::vector<std::string> statementDraftIds;
    std::string statementId;
};

} // namespace core::domain
