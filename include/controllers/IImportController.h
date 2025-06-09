#pragma once
#include <memory>
#include <string>

class IImportController {
public:
    virtual ~IImportController() = default;
    virtual std::shared_ptr<void> extractData(const std::string& filePath) = 0;
};