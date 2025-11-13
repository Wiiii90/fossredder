#include "pch.h"
#include "controllers/StatementController.h"
#include "models/StatementData.h"
#include "views/ConsoleView.h"
#include "services/IStatementExtractionService.h"
#include <filesystem>
#include <fstream>

std::shared_ptr<StatementData> StatementController::importStatement(const std::string& filePath) {
    ConsoleView consoleView;
    if (!std::filesystem::exists(filePath)) {
        consoleView.displayError("PDF datei existiert nicht: " + filePath);
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!extractionService_) {
        consoleView.displayError("No extraction service configured");
        throw std::runtime_error("No extraction service configured");
    }

    StatementExtractionRequest req; req.sourcePath = filePath;
    StatementExtractionResult res = extractionService_->extract(req);
    if (!res.data) {
        consoleView.displayError("Extraction failed");
        throw std::runtime_error("Extraction failed");
    }

    return res.data;
}