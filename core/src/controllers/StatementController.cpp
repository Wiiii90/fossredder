#include "core/pch.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include "core/models/Statement.h"
#include <iostream>

std::shared_ptr<Statement> StatementController::importStatement(const std::string& filePath, const std::string& runRoot, const std::string& runIdPrefix, std::function<void(double, const std::string&)> progressCallback, std::shared_ptr<std::atomic<bool>> cancelFlag) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!importService_) {
        throw std::runtime_error("No import service configured");
    }

    if (runRoot.empty()) {
        throw std::runtime_error("No import runRoot configured");
    }

    try {
        std::clog << "StatementController::importStatement - starting import: " << filePath << std::endl;
    } catch (...) {}

    ImportRequest req;
    req.sourcePath = filePath;
    req.runRoot = runRoot;
    req.runIdPrefix = runIdPrefix;
    req.progressCallback = std::move(progressCallback);
    req.cancelFlag = cancelFlag;
    ImportResult res = importService_->importStatement(req);

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