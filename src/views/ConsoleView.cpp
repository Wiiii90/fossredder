#include "pch.h"
#include "views/ConsoleView.h"
#include "models/StatementData.h"
#include <iostream>

void ConsoleView::displayPdfData(const std::shared_ptr<StatementData>& data) {
    if (!data) {
        std::cout << "[ConsoleView] Keine Daten zum Anzeigen." << std::endl;
        return;
    }

    const auto& render = data->getRenderArtifact();
    const auto& openCv = data->getOpenCvArtifacts();
    const auto& ocr = data->getOcrArtifacts();

    std::cout << "[ConsoleView] Source: " << data->getSourceFile() << std::endl;
    std::cout << "[ConsoleView] Poppler pages (images): " << render.images.size() << std::endl;
    std::cout << "[ConsoleView] OpenCV artifacts (pages): " << openCv.size() << std::endl;
    size_t totalFindings = 0;
    for (const auto &lo : openCv) totalFindings += lo.findings.size();
    std::cout << "[ConsoleView] OpenCV total findings: " << totalFindings << std::endl;

    std::cout << "[ConsoleView] OCR artifacts (pages): " << ocr.size() << std::endl;
    size_t totalOcrTables = 0;
    for (const auto &page : ocr) totalOcrTables += page.size();
    std::cout << "[ConsoleView] OCR total tables: " << totalOcrTables << std::endl;
}

void ConsoleView::displayError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}