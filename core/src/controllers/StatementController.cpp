#include "core/pch.h"
#include "core/controllers/StatementController.h"
#include "core/services/IStatementExtractionService.h"

std::shared_ptr<StatementData> StatementController::importStatement(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!extractionService_) {
        throw std::runtime_error("No extraction service configured");
    }

    StatementExtractionRequest req; req.sourcePath = filePath;
    StatementExtractionResult res = extractionService_->extract(req);
    if (!res.data) {
        throw std::runtime_error("Extraction failed");
    }

    return res.data;
}