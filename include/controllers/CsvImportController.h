#pragma once
#include "controllers/IImportController.h"
#include <memory>
#include <string>

class CsvImportController : public IImportController {
public:
    CsvImportController();
    std::shared_ptr<void> extractData(const std::string& filePath) override;
};