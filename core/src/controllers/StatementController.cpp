#include "core/pch.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include <iostream>

std::shared_ptr<StatementData> StatementController::importStatement(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!importService_) {
        throw std::runtime_error("No import service configured");
    }

    // Log before calling the import service
    try {
        std::clog << "StatementController::importStatement - starting import: " << filePath << std::endl;
    } catch (...) {}

    ImportRequest req; req.sourcePath = filePath;
    ImportResult res = importService_->importStatement(req);

    // Log after calling the import service
    try {
        std::clog << "StatementController::importStatement - import completed for: " << filePath
                  << ", data=" << (res.data ? "present" : "null")
                  << ", artifacts=" << res.artifacts.size() << std::endl;
    } catch (...) {}

    if (!res.data) {
        throw std::runtime_error("Extraction failed");
    }

    return res.data;
}