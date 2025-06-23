#pragma once
#include "controllers/IImportController.h"
#include <memory>
#include <string>
#include <vector>

class CsvImportController : public IImportController {
public:
    CsvImportController();
    std::shared_ptr<void> extractData(const std::string& filePath) override;
    std::vector<std::vector<std::string>> importCsv(const std::string& filePath, char delimiter = ';');
};