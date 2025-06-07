#include "pch.h"
#include "models/PdfExtractedData.h"
#include "views/ConsoleView.h"
#include "controllers/PdfImportController.h"
#include "poppler/PopplerPdfRenderer.h"
#include "ocr/TesseractOcrEngine.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    auto ocrEngine = std::make_shared<TesseractOcrEngine>();
    auto pdfRenderer = std::make_shared<PopplerPdfRenderer>();
    PdfImportController controller(ocrEngine, pdfRenderer);
    ConsoleView consoleView;

    try {
        auto extractedData = controller.extractData("C:\\coding\\fossredder\\Januar 2025.pdf");
        consoleView.displayPdfData(extractedData);
    } catch (const std::exception& e) {
        consoleView.displayError("Error during PDF processing: " + std::string(e.what()));
    }

    return 0;
}