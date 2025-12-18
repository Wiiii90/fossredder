#include "core/pch.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"

std::shared_ptr<StatementData> StatementController::importStatement(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!importService_) {
        throw std::runtime_error("No import service configured");
    }

    ImportRequest req; req.sourcePath = filePath;
    ImportResult res = importService_->importStatement(req);
    if (!res.data) {
        throw std::runtime_error("Extraction failed");
    }

    return res.data;
}